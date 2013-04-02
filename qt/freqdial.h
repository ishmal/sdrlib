#ifndef _FREQDIAL_H_
#define _FREQDIAL_H_

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

/**
 * A custom spinner-like frequency control
 */ 
class FreqDial : public QWidget
{
    Q_OBJECT

public:

    FreqDial(Sdr &parent) : par(parent)
        {
        //slow initialization, but very safe
        int i=0;
        digitStr[i]="0"; digitStrC[i++]="0,";
        digitStr[i]="1"; digitStrC[i++]="1,";
        digitStr[i]="2"; digitStrC[i++]="2,";
        digitStr[i]="3"; digitStrC[i++]="3,";
        digitStr[i]="4"; digitStrC[i++]="4,";
        digitStr[i]="5"; digitStrC[i++]="5,";
        digitStr[i]="6"; digitStrC[i++]="6,";
        digitStr[i]="7"; digitStrC[i++]="7,";
        digitStr[i]="8"; digitStrC[i++]="8,";
        digitStr[i]="9"; digitStrC[i++]="9,";
        }
        
    virtual ~FreqDial()
        {
        }
        
    double getFrequency()
        {
        double sum = 0.0;
        double power = 1.0;
        for (int i = 0 ; i < FREQ_DIGITS ; i++)
            {
            double d = (double) digits[i];
            sum += power * d;
            power *= 10.0;
            }
        return sum;
        }

    void setFrequency(double freq)
        {
        for (int i = 0 ; i < FREQ_DIGITS ; i++)
            digits[i] = 0;
        
        for (int i = 0 ; i < FREQ_DIGITS ; i++)
            {
            int dig = (int) fmod(freq,10.0);
            digits[i] = dig % 10;
            freq -= (double) dig;
            freq *= 0.1;
            if (freq <= 0.0)
                break;
            }
        repaint();
        }
        
signals:

    void frequencyChanged(double freq);

protected:

    virtual void paintEvent(QPaintEvent *event)
        {
        QPainter painter(this);
        int w = width();
        int h = height();
        QFont newFont("Arial", 20, QFont::Bold, true);
        painter.setFont(newFont);
        painter.fillRect(0,0,w,h,Qt::black);
        painter.setPen( Qt::green );
        int leading = 1;
        int digitWidth = w / FREQ_DIGITS;
        int idx = FREQ_DIGITS-1;
        for (int i=0 ; i < FREQ_DIGITS ; i++,idx--)
            {
            int digit = digits[idx];
            if (digit < 1 && leading)
                {

                }       
            else
                {
                leading = 0;
                //trace("idx:%d digit:%d", idx, digit);
                const char *str = (idx !=0 && (idx % 3)==0) ? digitStrC[digit] : digitStr[digit];
                painter.drawText(i*digitWidth, h/2, str);
                }
            }
        }
        
    virtual void wheelEvent(QWheelEvent *event)
        {
        int increment = event->delta();
        int updown = increment>0;
        int x = event->x();
        int idx = getIndex(x);
        //trace("incr:%d wheel:%d x:%d idx:%d", increment, updown,x,idx);
        if (updown)
            digitIncr(idx);
        else
            digitDecr(idx);
        emit frequencyChanged(getFrequency());
        repaint();
        }

    virtual void mousePressEvent(QMouseEvent *event)
        {
        int y = event->y();
        int updown = y < height()/2;
        int x = event->x();
        int idx = getIndex(x);
        if (updown)
            digitIncr(idx);
        else
            digitDecr(idx);
        emit frequencyChanged(getFrequency());
        repaint();
        }

    virtual void keyPressEvent(QKeyEvent * event)
        {
        int key = event->key();
        if (key == Qt::Key_Up)
            {
            //digitIncr(idx);
            repaint();
            }
        else if (key == Qt::Key_Down)
            {
            //digitDecr(idx);
            repaint();
            }
        }
        
        
private:

    static const int FREQ_DIGITS = 10;

    int digits[FREQ_DIGITS];
    const char *digitStr[10];
    const char *digitStrC[10];
    Sdr &par;

    void digitIncr(int pos)
        {
        if (pos < 0 || pos > FREQ_DIGITS-1)
            return;
        if (digits[pos] < 9)
            digits[pos]++;
        else
            {
            digits[pos] = 0;
            digitIncr(pos+1);
            }
        }

    void digitDecr(int pos)
        {
        if (pos < 0 || pos > FREQ_DIGITS-1)
            return;
        if (digits[pos] > 0)
            digits[pos]--;
        else
            {
            digits[pos] = 9;
            digitDecr(pos+1);
            }
        }

    int getIndex(int x)
        {
        int w = width();
        int h = height();
        int digitWidth = w / FREQ_DIGITS;
        return FREQ_DIGITS - 1 - x / digitWidth;
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
        fprintf(stderr, "FreqDial error:");
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
        fprintf(stderr, "\n");
        }

    void trace(const char *format, ...)
        {
        fprintf(stderr, "FreqDial:");
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
        fprintf(stderr, "\n");
        }

};




#endif /* _FREQDIAL_H_ */


