/***************************************************************************
                          enginerecord.h  -  description
                             -------------------
    copyright            : (C) 2007 by John Sully
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

#ifndef ENGINERECORD_H
#define ENGINERECORD_H

#include "controlobjectthread.h"
#include "configobject.h"
#include "engine/engineobject.h"
#include "writeaudiofile.h"

#define THRESHOLD_REC 2. //high enough that its not triggered by white noise

class ControlLogpotmeter;
class ConfigKey;
class ControlObject;

class EngineRecord : public EngineObject {
public:
    EngineRecord(ConfigObject<ConfigValue> *_config);
    ~EngineRecord();
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
private:
    WriteAudioFile *fOut;
    ConfigObject<ConfigValue> *config;
    ControlObjectThread* recReady;
    ControlObject* recReadyCO;
};

#endif
