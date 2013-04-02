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
    Waterfall *wf = (Waterfall *)ctx;
    wf->updatePs(ps, size);
}


Sdr::Sdr()
{
    sdr = sdrCreate();
    freqOffset = 0;
    ui.setupUi(this);
    waterfall = new Waterfall(*this);
    sdrSetPowerSpectrumFunc(sdr, powerSpectrumCallback, (void *)waterfall);
    ui.waterfallBox->addWidget(waterfall);
    freqDial = new FreqDial(*this);
    ui.freqDialBox->addWidget(freqDial);
    freqDial->setFrequency(123456789.0);
    connect(freqDial, SIGNAL(frequencyChanged(double)), this, SLOT(setCenterFrequency(double)));
    show();
}
        
Sdr::~Sdr()
{
   sdrDelete(sdr);
   delete waterfall;
}
        








