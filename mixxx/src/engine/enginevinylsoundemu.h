/***************************************************************************
                          enginevinylsoundemu.h  -  description
                             -------------------
    copyright            : (C) 2007 by Albert Santoni
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

#ifndef ENGINEVINYLSOUNDEMU_H
#define ENGINEVINYLSOUNDEMU_H

#include "engineobject.h"

class EngineVinylSoundEmu : public EngineObject
{
public:
    EngineVinylSoundEmu(ConfigObject<ConfigValue> *pConfig, const char *group);
    ~EngineVinylSoundEmu();
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
private:
    ConfigObject<ConfigValue> *m_pConfig;
    ControlObject *m_pRateEngine;
    float m_fAbsSpeed;
    float m_fGainFactor;
};

#endif
