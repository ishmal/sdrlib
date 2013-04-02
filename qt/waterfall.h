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
#include <QPaintEvent>
#include <QResizeEvent>
#include <QHoverEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>


#include <stdarg.h>
#include <cmath>
#include <sdrlib.h>


#include "sdr.h"
#include "freqdial.h"


class Waterfall : public QWidget
{
    Q_OBJECT

public:

    Waterfall(Sdr &parent) : par(parent)
        {
        resize(400, 300);
        image = QPixmap(width(), height());
        for (int i=0 ; i<256 ; i++)
            palette[i] = QColor::fromHsv(255-i, 255, 255, 255);
        vfoFreq = 0.0;
        }
        
    virtual ~Waterfall()
        {
        }
        
    /**
     * Receive a new line of power spectrum data
     */    
    void updatePs(unsigned int *ps, int size)
        {
        int w = image.width();
        int y = image.height() - 1;
        image.scroll(0, -1, 0, 1, w, y);
        QPainter painter(&image);
        int acc = -size;
        for (int x = 0; x < w ; x++)
            {
            while (acc < 0)
                {
                ps++;
                acc += w;
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
        
    double getVfoFreq()
        {
        return vfoFreq;
        }

    void setVfoFreq(double val)
        {
        vfoFreq = val;
        }

    double getPbLoFreq()
        {
        return pbLoFreq;
        }

    void setPbLoFreq(double val)
        {
        pbLoFreq = val;
        }

    double getPbHiFreq()
        {
        return pbHiFreq;
        }

    void setPbHiFreq(double val)
        {
        pbHiFreq = val;
        }

protected:

    virtual void paintEvent(QPaintEvent *event)
        {
        QPainter painter(this);
        painter.drawPixmap(0, 0, image);
        int w = width();
        int h = height();
        int center = w >> 1;
        painter.setPen(Qt::red);
        painter.drawLine(center, 0, center, h);
        }
        
    virtual void resizeEvent(QResizeEvent *event) 
        {
        QSize size = event->size();
        image = QPixmap(size.width(), size.height());
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
    QColor palette[256];
    double vfoFreq;
    double pbLoFreq;
    double pbHiFreq;
};



class Waterfall_orig : public QWidget
{
    Q_OBJECT

public:

    Waterfall_orig()
        {
        resize(400, 300);
        image = QImage(width(), height(), QImage::Format_RGB32);
        for (int i=0 ; i<256 ; i++)
            palette[i] = QColor::fromHsv(255-i, 255, 255, 255);
        }
        
    virtual ~Waterfall_orig()
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



#endif /* _WATERFALL_H_ */

