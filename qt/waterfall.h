#ifndef _WATERFALL_H_
#define _WATERFALL_H_

/**
 * Minimalist GUI for testing sdr functionality.
 * 
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2013 Bob Jamison
 * 
 *  This file is part of the SdrLib library.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <QtWidgets>
#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QColor>
#include <QCursor>


#include <stdarg.h>
#include <stdio.h>
#include <cmath>
#include <sdrlib.h>


#include "sdr.h"
#include "freqdial.h"



class Waterfall : public QWidget
{
    Q_OBJECT

public:

    static const int PS_HEIGHT=70;
    static const int LEGEND_HEIGHT=16;

    typedef enum { TUNE_NONE=0, TUNE_LO, TUNE_VFO, TUNE_HI, TUNE_LEGEND } TuneMode;

    Waterfall(Sdr &parent) : par(parent)
        {
        resize(400, 300);
        psImage = QPixmap(width(), PS_HEIGHT);
        wfImage = QPixmap(width(), height() - PS_HEIGHT - LEGEND_HEIGHT);
        legendImage = QPixmap(width(), LEGEND_HEIGHT);
        
        for (int i=0 ; i<256 ; i++)
            palette[i] = QColor::fromHsv(255-i, 255, 255, 255);
        pbCol = QColor(255, 255, 255, 100); 
        vfoFreq   =      0.0;
        pbLo   = -50000.0;
        pbHi   =  50000.0;
        hoverMode = TUNE_NONE;
        tuneMode  = TUNE_NONE;
        setMouseTracking(true);
        setFocusPolicy(Qt::StrongFocus);
        psGain = 1.0;
        zoomLevel = 1;
        zoomScale = 1.0;
        legendFont = QFont("Ariel", 8, QFont::SansSerif, true);
        adjust();
        }
        
    virtual ~Waterfall()
        {
        }
        
    /**
     * Receive a new line of power spectrum data.  Use Bresenham's algorithm
     * to scale the data according to the zoom level and the widget size.
     */    
    void updatePs(unsigned int *powerSpectrum, int size)
        {
        int nrSamples = size * zoomScale;
        int startIndex = (size - nrSamples) >> 1;
        unsigned int *ps = powerSpectrum + startIndex;
        unsigned int *out = scaledPs;
        int w = width();
        int h = height();
        if (nrSamples > w)
            {
            int acc = -nrSamples;
            for (int x = 0; x < w ; x++)
                {
                while (acc < 0)
                    {
                    ps++;
                    acc += w;
                    }
                acc -= nrSamples;
                *out++ = *ps * psGain;
                }
            }
        else
            {
            int acc = -w;
            for (int x = 0; x < w ; x++)
                {
                acc += nrSamples;
                if (acc >= 0)
                    {
                    *out++ = (*ps++) * psGain;
                    acc -= w;
                    }
                }
            }
        updateWaterfall(scaledPs, size);
        updatePowerSpectrum(scaledPs, size);
        update();
        }
        
    void sayHello(QString msg)
        {
        qDebug() << msg ;
        }
        
    void setVfoFreq(float val)
        {
        vfoFreq = val;
        emit frequenciesChanged(vfoFreq, pbLo, pbHi);
        update();
        }

    float getVfoFreq()
        {
        return vfoFreq;
        }

    void setPbLo(double val)
        {
        if (val < pbHi)
            {
            pbLo = val;
            status("Bw: %f", pbHi - pbLo);
            emit frequenciesChanged(vfoFreq, pbLo, pbHi);
            update();
            }
        }

    double getPbLo()
        {
        return pbLo;
        }

    void setPbHi(double val)
        {
        if (val > pbLo)
            {
            pbHi = val;
            status("Bw: %f", pbHi - pbLo);
            emit frequenciesChanged(vfoFreq, pbLo, pbHi);
            update();
            }
        }

    double getPbHi()
        {
        return pbHi;
        }
        
    void setPsGain(float val)
        {
        psGain = 5.0 * val;
        }
        
    /**
     * Here we adjust the zoom level.  We also calculate the "zoomScale", the
     * actual value we multiply by the sampleRate and/or N to get our window.
     */         
    void setZoomLevel(int level)
        {
        if (level >= 0 && level <=10)
            {
            zoomLevel = level;
            zoomScale = 1.0 / (1<<level);
            }
        adjust();
        update();
        }

    void adjust()
        {
        float fs = par.getSampleRate();
        float cf = par.getCenterFrequency();
        float hzWidth = fs * zoomScale;
        float hzPerPixel = hzWidth / width();
        //Redraw the legend
        float hpplog = ceil(log10(hzPerPixel))+ 1.0;
        float tickScale = pow(10.0, hpplog);
        float minFreq = cf - hzWidth * 0.5;
        float maxFreq = cf + hzWidth * 0.5;
        float firstTick = ceil(minFreq / tickScale) * tickScale;
        float f = firstTick;
        int w = legendImage.width();
        int h = legendImage.height();
        QPainter painter(&legendImage);
        painter.setFont(legendFont);
        painter.fillRect(0, 0, w, h, Qt::black);
        painter.setPen(Qt::green);
        int tickNr = (int) (firstTick / tickScale);
        while (f < maxFreq)
            {
            int x = freqToX(f - cf);
            if (tickNr % 10 == 0)
                {
                formatFreq(legendBuf, 32, f);
                int stringWidth = painter.fontMetrics().width(legendBuf);
                painter.drawText(x-stringWidth/2, h-7, legendBuf);
                painter.drawLine(x, h-6, x, h);
                } 
            else if (tickNr % 5 == 0)
                painter.drawLine(x, h-4, x, h); 
            else
                painter.drawLine(x, h-2, x, h); 
            f += tickScale;
            tickNr++;
            }
        //trace("zl:%d hz:%f hpp:%f ts:%f mf:%f firstTick:%f", zoomLevel, hzWidth, hzPerPixel, tickScale, minFreq, firstTick);
 
        //Check to see if VFO is outside tuning area
        float minVfo = -hzWidth * 0.45;
        float maxVfo =  hzWidth * 0.45;
        if (vfoFreq < minVfo)
            setVfoFreq(minVfo);
        else if (vfoFreq > maxVfo)
            setVfoFreq(maxVfo);
        }

signals:

    void frequenciesChanged(float vfoFreq, float pbLo, float pbHi);


protected:

    virtual void paintEvent(QPaintEvent *event)
        {
        QPainter painter(this);
        int w = width();
        int h = height();
        drawWaterfall(painter, w, h);
        drawReticle(painter, w, h);
        drawLegend(painter, w, h);
        }
        
    virtual void resizeEvent(QResizeEvent *event) 
        {
        QSize size = event->size();
        psImage = QPixmap(size.width(), PS_HEIGHT);
        wfImage = QPixmap(size.width(), size.height() - PS_HEIGHT - LEGEND_HEIGHT);
        legendImage = QPixmap(size.width(), LEGEND_HEIGHT);
        adjust();
        }

    virtual void wheelEvent(QWheelEvent *event)
        {
        }

    virtual void mousePressEvent(QMouseEvent *event)
        {
        //very simple
        tuneMode     = hoverMode;
        mouseDownX   = event->pos().x();
        mouseDownCf  = par.getCenterFrequency();
        mouseDownLo  = pbLo;
        mouseDownVfo = vfoFreq;
        mouseDownHi  = pbHi;
        }

    virtual void mouseMoveEvent(QMouseEvent *event)
        {
        int x = event->pos().x();
        int y = event->pos().y();
        float fw = (float)width();
        float fx = (float)(x - mouseDownX);
        float freqDiff =  fx/fw * par.getSampleRate() * zoomScale;
        switch (tuneMode)
            {
            case TUNE_LO:
                {
                setPbLo(mouseDownLo + freqDiff);
                break;
                }
            case TUNE_VFO:
                {
                setVfoFreq(mouseDownVfo + freqDiff);
                break;
                }
            case TUNE_HI:
                {       
                setPbHi(mouseDownHi + freqDiff);
                break;
                }
            case TUNE_LEGEND:
                {      
                par.setCenterFrequency(mouseDownCf - freqDiff);  //other direction
                break;
                }
            default:
                {
                TuneMode currMode = getTuneMode(x, y);
                if (currMode == hoverMode)
                    return;
                hoverMode = currMode;
                switch (hoverMode)
                    {
                    case TUNE_LO : 
                        setCursor(Qt::SizeHorCursor);
                        break;
                    case TUNE_VFO : 
                        setCursor(Qt::SizeAllCursor);
                        break;
                    case TUNE_HI : 
                        setCursor(Qt::SizeHorCursor);
                        break;
                    case TUNE_LEGEND : 
                        setCursor(Qt::ClosedHandCursor);
                        break;
                    default : 
                        setCursor(Qt::ArrowCursor);
                    }
                }
            }
        }

    virtual void mouseReleaseEvent(QMouseEvent *event)
        {
        tuneMode = TUNE_NONE;
        }

    virtual void keyPressEvent(QKeyEvent *event)
        {
        switch (event->key())
            {
            case Qt::Key_Up:
                adjustZoomLevel(true);
                break;
            case Qt::Key_Down:
                adjustZoomLevel(false);
                break;
            default :
                break;
            }
        }

 
 
private:

    void drawWaterfall(QPainter &painter, int w, int h)
        {
        painter.drawPixmap(0, 0, psImage);
        painter.drawPixmap(0, PS_HEIGHT, wfImage);
        }

    void drawReticle(QPainter &painter, int w, int h)
        {
        int center = w >> 1;
        painter.setPen(Qt::red);
        painter.drawLine(center, 0, center, h);
        int pbLoX = freqToX(vfoFreq + pbLo);
        int pbHiX = freqToX(vfoFreq + pbHi);
        painter.fillRect(pbLoX, 0, pbHiX-pbLoX, h, pbCol);
        int vfoX = freqToX(vfoFreq);
        painter.setPen(Qt::green);
        painter.drawLine(vfoX, 0, vfoX, h);
        }

    void drawLegend(QPainter &painter, int w, int h)
        {
        painter.drawPixmap(0, h - LEGEND_HEIGHT, legendImage);
        }
        
        
    /**
     * Receive a new line of power spectrum data
     */    
    void updateWaterfall(unsigned int *powerSpectrum, int size)
        {
        unsigned int *ps = powerSpectrum;
        int w = wfImage.width();
        int h = wfImage.height();
        wfImage.scroll(0, 1, 0, 0, w, h-1);
        QPainter painter(&wfImage);
        for (int x = 0 ; x < w ; x++)
            {
            unsigned int v  = *ps++;
            QColor col = palette[v>>1 & 255];
            painter.setPen(col);
            painter.drawPoint(x,0);
            }
        }
        

    /**
     * Receive a new line of power spectrum data
     */    
    void updatePowerSpectrum(unsigned int *powerSpectrum, int size)
        {
        unsigned int *ps = powerSpectrum;
        float *avg = psAvg;
        int w = psImage.width();
        int h = psImage.height();
        float fh = (float)h;
        QPainter painter(&psImage);
        painter.fillRect(0, 0, w, h, Qt::black);
        QPainterPath path;
        path.moveTo(0.0, fh);
        for (int x=0 ; x < w ; x++)
            {
            unsigned int v  = (*ps++) >> 2;
            float hpos = fh - v;
            *avg = *avg*0.8 + hpos * 0.2;
            path.lineTo((float)x, *avg);
            avg++;
            }
        path.lineTo((float)w, fh);
        path.moveTo(0.0, fh);
        path.closeSubpath();
        painter.setPen(Qt::green);
        painter.drawPath(path);
        }
        

    void formatFreq(char *buf, int buflen, float f)
        {
        if (f > 1000000.0)
            snprintf(buf, buflen-1, "%.6fM", f*.000001);
        else if (f > 1000.0)
            snprintf(buf, buflen-1, "%.3fk", f*.001);
        else
            snprintf(buf, buflen-1, "%.0f", f);
        }
        
        
    void adjustZoomLevel(bool up)
        {
        if (up)
            {
            setZoomLevel(zoomLevel + 1);
            }
        else
            {
            setZoomLevel(zoomLevel - 1);
            }
        }

    TuneMode getTuneMode(int x, int y)
        {
        if (y > height() - LEGEND_HEIGHT)
            {
            return TUNE_LEGEND;
            }
        else
            {
            int vfoX  = freqToX(vfoFreq);
            if (x >= vfoX-25 && x <= vfoX-5)
                return TUNE_LO;
            else if (x >= vfoX+5 && x <= vfoX+25)
                return TUNE_HI;
            else if (x >= vfoX-5 && x <= vfoX+5)
                return TUNE_VFO;
            else
                return TUNE_NONE;
            }
        }

    float xToFreq(int x)
        {
        float fw = (float)width();
        float fx = (float)x;
        float proportion = fx/fw;
        float pos = proportion - 0.5;
        float f = pos * par.getSampleRate() * zoomScale;
        //status("fw:%f fx:%f pos:%f vfo:%f\n", fw, fx, pos, f);
        return f;
        }


    int freqToX(float freq)
        {
        float proportion = freq / par.getSampleRate() / zoomScale;
        float pos = proportion + 0.5;
        int x = (int)(pos * (float)width());
        return x;
        }
        
    void status(const char *format, ...)
        {
        va_list args;
        va_start(args, format);
        par.status(format, args);
        va_end(args);
        }


    void error(const char *format, ...)
        {
        fprintf(stderr, "Waterfall error:");
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
        fprintf(stderr, "\n");
        }

    void trace(const char *format, ...)
        {
        fprintf(stderr, "Waterfall:");
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
        fprintf(stderr, "\n");
        }

    Sdr &par;
    QPixmap wfImage;
    QPixmap psImage;
    QPixmap legendImage;
    QColor palette[256];
    QColor pbCol;
    float vfoFreq;
    float pbLo;
    float pbHi;
    TuneMode hoverMode;
    TuneMode tuneMode;
    int mouseDownX;
    float mouseDownCf;
    float mouseDownLo;
    float mouseDownVfo;
    float mouseDownHi;
    int zoomLevel;
    float zoomScale;
    QFont legendFont;
    char legendBuf[32];
    unsigned int scaledPs[100000];
    float psAvg[100000];
    float psGain;
};






















#if 0


class Waterfall : public QWidget
{
    Q_OBJECT

public:

    Waterfall()
        {
        resize(400, 300);
        image = QImage(width(), height(), QImage::Format_RGB32);
        for (int i=0 ; i<256 ; i++)
            palette[i] = QColor::fromHsv(255-i, 255, 255, 255);
        }
        
    virtual ~Waterfall()
        {
        }
        
    /**
     * Receive a new line of power spectrum data
     */    
    void updatePs(unsigned int *ps, int size)
        {
        int width = image.width();
        QPainter painter(&image);
        int y = image.height() - 1;
        int byteWidth = image.bytesPerLine();
        int byteCount = image.byteCount();
        uchar *buf = (uchar *)image.constBits();
        memmove(buf, buf+byteWidth, byteCount-byteWidth);
        int acc = -size;
        for (int x = 0; x < width ; x++)
            {
            while (acc < 0)
                {
                ps++;
                acc += width;
                }
            acc -= size;
            unsigned int v  = *ps;
            QColor col = palette[v>>1 & 255];
            painter.setPen(col);
            painter.drawPoint(x,y);
            }
        update();
        }
        
    void sayHello(QString msg)
        {
        qDebug() << msg ;
        }

protected:

    virtual void paintEvent(QPaintEvent *event)
        {
        QPainter painter(this);
        painter.drawImage(0, 0, image);
        int w = width();
        int h = height();
        int center = w >> 1;
        painter.setPen(Qt::red);
        painter.drawLine(center, 0, center, h);
        }
        
    virtual void resizeEvent(QResizeEvent *event) 
        {
        QSize size = event->size();
        image = QImage(size.width(), size.height(), QImage::Format_RGB32);
        }

    virtual void hoverMoveEvent(QHoverEvent *event)
        {
        }

    virtual void mouseMoveEvent(QMouseEvent *event)
        {
        }

    virtual void wheelEvent(QWheelEvent *event)
        {
        }

    virtual void mousePressEvent(QMouseEvent *event)
        {
        trace("Click!");
        }

    virtual void mouseReleaseEvent(QMouseEvent *event)
        {
        }

 
 
private:

    void error(const char *format, ...)
        {
        fprintf(stderr, "Waterfall error:");
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
        fprintf(stderr, "\n");
        }

    void trace(const char *format, ...)
        {
        fprintf(stderr, "Waterfall:");
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
        fprintf(stderr, "\n");
        }

    QImage image;
    QColor palette[256];
    
};
#endif


#endif /* _WATERFALL_H_ */

