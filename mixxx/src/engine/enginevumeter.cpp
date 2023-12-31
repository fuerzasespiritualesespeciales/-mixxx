/***************************************************************************
                          enginevumeter.cpp  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                :
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifdef __WINDOWS__
#pragma intrinsic(fabs)
#endif

#include "enginevumeter.h"
#include "controlpotmeter.h"

EngineVuMeter::EngineVuMeter(const char * group)
{
    // The VUmeter widget is controlled via a controlpotmeter, which means
    // that it should react on the setValue(int) signal.
    m_ctrlVuMeter = new ControlPotmeter(ConfigKey(group, "VuMeter"), 0., 1.);
    m_ctrlVuMeter->set(0);
    // left channel VU meter
    m_ctrlVuMeterL = new ControlPotmeter(ConfigKey(group, "VuMeterL"), 0., 1.);
    m_ctrlVuMeterL->set(0);
    // right channel VU meter
    m_ctrlVuMeterR = new ControlPotmeter(ConfigKey(group, "VuMeterR"), 0., 1.);
    m_ctrlVuMeterR->set(0);

    // Initialize the calculation:
    m_iSamplesCalculated = 0;
    m_fRMSvolumeL = 0;
    m_fRMSvolumeSumL = 0;
    m_fRMSvolumeR = 0;
    m_fRMSvolumeSumR = 0;
}

EngineVuMeter::~EngineVuMeter()
{
    delete m_ctrlVuMeter;
    delete m_ctrlVuMeterL;
    delete m_ctrlVuMeterR;
}

void EngineVuMeter::process(const CSAMPLE * pIn, const CSAMPLE *, const int iBufferSize)
{
    // Calculate the summed absolute volume
    for (int i=0; i<iBufferSize/2; ++i)
    {
        m_fRMSvolumeSumL += fabsf(pIn[2*i]);
        m_fRMSvolumeSumR += fabsf(pIn[2*i+1]);
    }

    m_iSamplesCalculated += iBufferSize/2;

    // Are we ready to update the VU meter?:
    if (m_iSamplesCalculated > (44100/2/UPDATE_RATE) )
    {
        doSmooth(m_fRMSvolumeL, log10(m_fRMSvolumeSumL/(m_iSamplesCalculated*1000)+1));
	doSmooth(m_fRMSvolumeR, log10(m_fRMSvolumeSumR/(m_iSamplesCalculated*1000)+1));

	m_ctrlVuMeterL->set(m_fRMSvolumeL);
        m_ctrlVuMeterR->set(m_fRMSvolumeR);
        m_ctrlVuMeter->set( (m_fRMSvolumeL + m_fRMSvolumeR) / 2);

	    // Reset calculation:
        m_iSamplesCalculated = 0;
        m_fRMSvolumeSumL = 0;
        m_fRMSvolumeSumR = 0;
    }
}


 void EngineVuMeter::doSmooth(FLOAT_TYPE &currentVolume, FLOAT_TYPE newVolume)
{
	if (currentVolume > newVolume)
		currentVolume -= DECAY_SMOOTHING * (currentVolume - newVolume);
	else
		currentVolume += ATTACK_SMOOTHING * (newVolume - currentVolume);
	if (currentVolume < 0)
		currentVolume=0;
	if (currentVolume > 1.0)
		currentVolume=1.0;
}
