#ifndef _SDR_H_
#define _SDR_H_

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


#include <stdarg.h>
#include <sdrlib.h>

#include "ui_simple.h"

class Waterfall; 
class FreqDial; 
 
class Sdr : public QMainWindow
{
    Q_OBJECT

public:

    Sdr();
        
    virtual ~Sdr();
    
    //#################################################################
    //# M E T H O D S
    //# Will only need to move members to .cpp if there are circular refs
    //# to its children. (dial, waterfall, etc)
    //#################################################################
        
    void status(const char *format, va_list args)
        {
        vsnprintf(statbuf, STATBUFSIZE, format, args);
        ui.statusbar->showMessage(statbuf);
        }
    
    void status(const char *format, ...)
        {
        va_list args;
        va_start(args, format);
        status(format, args);
        va_end(args);
        }
    
    
    float getSampleRate()
        {
        return sdrGetSampleRate(sdr);
        }
    
    int setSampleRate(float rate)
        {
        return sdrSetSampleRate(sdr, rate);
        }
        
    void adjust();


    //#################################################################
    //# S L O T S
    //#################################################################

public slots:

    double getCenterFrequency()
        {
        return sdrGetCenterFrequency(sdr) - freqOffset;
        }
    
    /**
     * References waterfall and dial.  Look in .cpp
     */
    void setCenterFrequency(double freq);
        
    void setDdcFreqs(float vfoFreqParm, float pbLoOff, float pbHiOff)
        {
        vfoFreq = vfoFreqParm;
        sdrSetDdcFreqs(sdr, vfoFreq, pbLoOff, pbHiOff);
        showVfoFreq();
        status("ddc: %f %f %f", vfoFreq, pbLoOff, pbHiOff);
        }
    
    void startStop()
        {
        if (ui.startStopBtn->isChecked())
            {
            sdrStart(sdr);
            adjust();
            status("start");
            }
        else
            {
            sdrStop(sdr);
            adjust();
            status("stop");
            }
        }

    void adjustRfGain(int gain)
        {
        float fgain = ((float)gain) / 100.0;
        status("rf gain:%f", fgain);
        sdrSetRfGain(sdr, fgain);
        }

    void adjustAfGain(int gain)
        {
        float fgain = ((float)gain) / 100.0;
        status("af gain:%f", fgain);
        sdrSetAfGain(sdr, fgain);
        }

    void adjustFreqOffset(double val)
        {
        freqOffset = val;
        status("offset:%f", freqOffset);
        }

    /**
     * References waterfall.  Look in .cpp
     */
    void adjustPsGain(int scale);

    /**
     * References waterfall.  Look in .cpp
     */
    void adjustPsZoom(int zoomLevel);

    void modeAm(bool val)
        {
        status("am:%d", val);
        sdrSetMode(sdr, MODE_AM);
        }

    void modeFm(bool val)
        {
        status("fm:%d", val);
        sdrSetMode(sdr, MODE_FM);
        }

    void modeLsb(bool val)
        {
        status("lsb:%d", val);
        sdrSetMode(sdr, MODE_LSB);
        }

    void modeUsb(bool val)
        {
        status("usb:%d", val);
        sdrSetMode(sdr, MODE_USB);
        }
        
protected:

    virtual void closeEvent(QCloseEvent *event)
        {
        sdrStop(sdr);
        sdrDelete(sdr);
        }


private:

    void showVfoFreq()
        {
        char freqstr[32];
        snprintf(freqstr, 31, "%ld", (long)(getCenterFrequency() + vfoFreq));
        ui.vfoLabel->setText(freqstr);
        }


    double freqOffset;
    Ui_MainWindow ui;
    SdrLib *sdr;
    Waterfall *waterfall;
    FreqDial *freqDial;
    float vfoFreq;
    static const int STATBUFSIZE = 256;
    char statbuf[STATBUFSIZE + 1];

};


#endif /* _SDR_H_ */


