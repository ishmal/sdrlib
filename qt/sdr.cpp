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
#include "waterfall.h"
#include "freqdial.h"


static void powerSpectrumCallback(unsigned int *ps, int size, void *ctx)
{
    Sdr *sdr = (Sdr *) ctx;
    sdr->updatePowerSpectrum(ps, size);
}


Sdr::Sdr()
{
    sdr = sdrCreate((void *)this, powerSpectrumCallback);
    freqOffset = 0;
    ui.setupUi(this);
    waterfall = new Waterfall(*this);
    ui.waterfallBox->addWidget(waterfall);
    connect(waterfall, SIGNAL(frequenciesChanged(float,float,float)), this, SLOT(setDdcFreqs(float,float,float)));
    freqDial = new FreqDial(*this);
    ui.freqDialBox->addWidget(freqDial);
    double freq = 88700000.0;
    freqDial->setFrequency(freq);
    sdrSetCenterFrequency(sdr, freq);
    connect(freqDial, SIGNAL(frequencyChanged(double)), this, SLOT(setCenterFrequency(double)));
    show();
}
        
Sdr::~Sdr()
{
   sdrDelete(sdr);
   delete freqDial;
   delete waterfall;
}


void Sdr::adjust()
{
    waterfall->adjust();
}

void Sdr::setCenterFrequency(double freq)
{
    sdrSetCenterFrequency(sdr, freq - freqOffset);
    freqDial->setFrequency(freq);
    showVfoFreq();
    showCfFreq();
    adjust();
    status("freq: %f", freq);
}

        
void Sdr::adjustPsGain(int scale)
{
    float fscale = ((float)scale) / 100.0;
    waterfall->setPsGain(fscale);
    status("ps scale:%f", fscale);
}

void Sdr::adjustPsZoom(int zoom)
{
    waterfall->setZoomLevel(zoom);
    status("ps zoom:%d", zoom);
}

void Sdr::updatePowerSpectrum(unsigned int *ps, int size)
{
    waterfall->updatePs(ps, size);
}







