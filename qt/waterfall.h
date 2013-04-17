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

    static const int LEGEND_HEIGHT=16;

    typedef enum { TUNE_NONE=0, TUNE_LO, TUNE_VFO, TUNE_HI } TuneMode;

    Waterfall(Sdr &parent) : par(parent)
        {
        resize(400, 300);
        image = QPixmap(width(), height() - LEGEND_HEIGHT);
        legendImage = QPixmap(width(), LEGEND_HEIGHT);
        for (int i=0 ; i<256 ; i++)
            palette[i] = QColor::fromHsv(255-i, 255, 255, 255);
        pbCol = QColor(255, 255, 255, 100); 
        vfoFreq   =      0.0;
        pbLoOff   = -50000.0;
        pbHiOff   =  50000.0;
        hoverMode = TUNE_NONE;
        tuneMode  = TUNE_NONE;
        setMouseTracking(true);
        setFocusPolicy(Qt::StrongFocus);
        zoomLevel = 1;
        legendFont = QFont("Courier", 8, QFont::Bold, true);
        adjust();
        }
        
    virtual ~Waterfall()
        {
        }
        
    /**
     * Receive a new line of power spectrum data
     */    
    void updatePs(unsigned int *powerSpectrum, int size)
        {
        int nrSamples = size / zoomLevel;
        int startIndex = (size - nrSamples) >> 1;
        unsigned int *ps = powerSpectrum + startIndex;
        int w = image.width();
        int y = image.height() - 1;
        image.scroll(0, -1, 0, 1, w, y);
        QPainter painter(&image);
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
                unsigned int v  = *ps;
                QColor col = palette[v>>1 & 255];
                painter.setPen(col);
                painter.drawPoint(x,y);
                }
            }
        else
            {
            int acc = -w;
            unsigned int v = *ps++;
            for (int x = 0; x < w ; x++)
                {
                acc += nrSamples;
                if (acc >= 0)
                    {
                    v = *ps++;
                    acc -= w;
                    }
                QColor col = palette[v>>1 & 255];
                painter.setPen(col);
                painter.drawPoint(x,y);
                }
            }
        update();
        }
        
    void sayHello(QString msg)
        {
        qDebug() << msg ;
        }
        
    void setVfoFreq(double val)
        {
        vfoFreq = val;
        emit frequenciesChanged(vfoFreq, pbLoOff, pbHiOff);
        update();
        }

    double getVfoFreq()
        {
        return vfoFreq;
        }

    void setPbLoOff(double val)
        {
        if (val < pbHiOff)
            {
            pbLoOff = val;
            status("Bw: %f", pbHiOff - pbLoOff);
            emit frequenciesChanged(vfoFreq, pbLoOff, pbHiOff);
            update();
            }
        }

    double getPbLoOff()
        {
        return pbLoOff;
        }

    void setPbHiOff(double val)
        {
        if (val > pbLoOff)
            {
            pbHiOff = val;
            status("Bw: %f", pbHiOff - pbLoOff);
            emit frequenciesChanged(vfoFreq, pbLoOff, pbHiOff);
            update();
            }
        }

    double getPbHiOff()
        {
        return pbHiOff;
        }

    void adjust()
        {
        float fs = par.getSampleRate();
        float cf = par.getCenterFrequency();
        float hzWidth = fs / zoomLevel;
        float hzPerPixel = hzWidth / width();
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
        }

signals:

    void frequenciesChanged(float vfoFreq, float pbLoOff, float pbHiOff);


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
        image = QPixmap(size.width(), size.height()-LEGEND_HEIGHT);
        legendImage = QPixmap(size.width(), LEGEND_HEIGHT);
        adjust();
        }

    virtual void wheelEvent(QWheelEvent *event)
        {
        }

    virtual void mousePressEvent(QMouseEvent *event)
        {
        //very simple
        tuneMode = hoverMode;
        }

    virtual void mouseMoveEvent(QMouseEvent *event)
        {
        int x = event->pos().x();
        float freq = xToFreq(x);
        switch (tuneMode)
            {
            case TUNE_LO:
                {
                setPbLoOff(freq - vfoFreq);
                break;
                }
            case TUNE_VFO:
                {
                setVfoFreq(freq);
                break;
                }
            case TUNE_HI:
                {       
                setPbHiOff(freq - vfoFreq);
                break;
                }
            default:
                {
                TuneMode currMode = getTuneMode(x);
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
        painter.drawPixmap(0, 0, image);
        }

    void drawReticle(QPainter &painter, int w, int h)
        {
        int center = w >> 1;
        painter.setPen(Qt::red);
        painter.drawLine(center, 0, center, h);
        int pbLoX = freqToX(vfoFreq + pbLoOff);
        int pbHiX = freqToX(vfoFreq + pbHiOff);
        painter.fillRect(pbLoX, 0, pbHiX-pbLoX, h, pbCol);
        int vfoX = freqToX(vfoFreq);
        painter.setPen(Qt::green);
        painter.drawLine(vfoX, 0, vfoX, h);
        }

    void drawLegend(QPainter &painter, int w, int h)
        {
        painter.drawPixmap(0, h - LEGEND_HEIGHT, legendImage);
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
        
        
    /**
     * Here we adjust the zoom level.  We also calculate the "zoomScale", the
     * closest power of 10 above the number of hertz per pixel.  This is
     * the finest granularity of tickmarks possible.
     */         
    void adjustZoomLevel(bool up)
        {
        if (up)
            {
            if (zoomLevel < 512)
                zoomLevel <<= 1;
            }
        else
            {
            if (zoomLevel > 1)
                zoomLevel >>= 1;
            }
        adjust();
        update();
        }

    TuneMode getTuneMode(int x)
        {
        int pbLoX = freqToX(vfoFreq + pbLoOff);
        int vfoX  = freqToX(vfoFreq);
        int pbHiX = freqToX(vfoFreq + pbHiOff);
        if (x >= vfoX-4 && x <= vfoX+4)
            return TUNE_VFO;
        else if (x >= pbLoX-3 && x <= pbLoX+3)
            return TUNE_LO;
        else if (x >= pbHiX-3 && x <= pbHiX+3)
            return TUNE_HI;
        else
            return TUNE_NONE;
        }

    float xToFreq(int x)
        {
        float fw = (float)width();
        float fx = (float)x;
        float proportion = fx/fw;
        float pos = proportion - 0.5;
        float f = pos * par.getSampleRate() / (float) zoomLevel;
        //status("fw:%f fx:%f pos:%f vfo:%f\n", fw, fx, pos, f);
        return f;
        }


    int freqToX(float freq)
        {
        float proportion = freq / par.getSampleRate() * (float) zoomLevel;
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
    QPixmap image;
    QPixmap legendImage;
    QColor palette[256];
    QColor pbCol;
    double vfoFreq;
    double pbLoOff;
    double pbHiOff;
    TuneMode hoverMode;
    TuneMode tuneMode;
    bool dragging;
    int zoomLevel;
    QFont legendFont;
    char legendBuf[32];
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

