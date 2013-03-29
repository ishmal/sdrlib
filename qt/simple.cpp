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

#include "ui_simple.h"




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
        memmove(buf, buf+byteWidth, byteCount);
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


#define FREQ_DIGITS 10

class FreqDial : public QWidget
{
    Q_OBJECT

public:

    FreqDial()
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
            sum += power * (double) digits[i];
            power *= 10.0;
            }
        return sum;
        }

    void setFrequency(double freq)
        {
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
        QFont newFont("Courier", 20, QFont::Bold, true);
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
        int increment = event->pixelDelta().y();
        int updown = increment>0;
        int x = event->x();
        int idx = getIndex(x);
        //trace("wheel:%d x:%d idx:%d", updown,x,idx);
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

    int digits[FREQ_DIGITS];
    const char *digitStr[10];
    const char *digitStrC[10];

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


 
 
static void powerSpectrumCallback(unsigned int *ps, int size, void *ctx)
{
    Waterfall *wf = (Waterfall *)ctx;
    wf->updatePs(ps, size);
}
 
 
 
class Sdr : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(double centerFrequency READ getCenterFrequency WRITE setCenterFrequency)
    Q_PROPERTY(float gain READ getGain WRITE setGain)


public:

    Sdr() : waterfall(NULL)
        {
        sdr = sdrCreate();
        freqOffset = 0;
        ui.setupUi(this);
        waterfall = new Waterfall();
        sdrSetPowerSpectrumFunc(sdr, powerSpectrumCallback, (void *)waterfall);
        ui.waterfallBox->addWidget(waterfall);
        freqDial = new FreqDial();
        ui.freqDialBox->addWidget(freqDial);
        freqDial->setFrequency(123456789.0);
        connect(freqDial, SIGNAL(frequencyChanged(double)), this, SLOT(setCenterFrequency(double)));
        show();
        }
        
    virtual ~Sdr()
        {
        sdrDelete(sdr);
        delete waterfall;
        }
        
public slots:

    double getCenterFrequency()
        {
        return sdrGetCenterFrequency(sdr);
        }
    
    void setCenterFrequency(double freq)
        {
        sdrSetCenterFrequency(sdr, freq);
        status("freq: %f", freq);
        }
    
    float getGain()
        {
        return sdrGetGain(sdr);
        }
    
    void setGain(float gain)
        {
        sdrSetGain(sdr, gain);
        }
        
    Q_INVOKABLE void start()
        {
        sdrStart(sdr);
        }
    
    Q_INVOKABLE void stop()
        {
        sdrStop(sdr);
        }
    
        
    Q_INVOKABLE void sayHello(QString msg)
        {
        qDebug() << msg ;
        }

    void startStop()
        {
        if (ui.startStopBtn->isChecked())
            start();
        else
            stop();
        }

    void adjustGain(int gain)
        {
        float fgain = ((float)gain) / 100.0;
        status("gain:%f", fgain);
        sdrSetGain(sdr, fgain);
        }

    void adjustFreqOffset(double val)
        {
        freqOffset = val;
        status("offset:%f", freqOffset);
        }

    void modeAm(bool val)
        {
        status("am:%d", val);
        }

    void modeFm(bool val)
        {
        status("fm:%d", val);
        }

    void modeLsb(bool val)
        {
        status("lsb:%d", val);
        }

    void modeUsb(bool val)
        {
        status("usb:%d", val);
        }


private:

    #define STATBUFSIZE 128
    char statbuf[STATBUFSIZE + 1];
    void status(const char *format, ...)
        {
        va_list args;
        va_start(args, format);
        vsnprintf(statbuf, STATBUFSIZE, format, args);
        ui.statusbar->showMessage(statbuf);
        va_end(args);
        }

    double freqOffset;
    Ui_MainWindow ui;
    SdrLib *sdr;
    Waterfall *waterfall;
    FreqDial *freqDial;

};

#include "simple.moc"



/**
 * This is the "main" of the gui app
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    new Sdr();
    return app.exec();
}
 
 