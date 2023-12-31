/***************************************************************************
               engineabstractrecord.h  -  Abstract Recording class
                             -------------------
    copyright            : (C) 2007 by Wesley Stessens
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENGINEABSTRACTRECORD_H
#define ENGINEABSTRACTRECORD_H

#include "defs.h"
#include "engineobject.h"

class EngineAbstractRecord : public EngineObject {
public:
    virtual void writePage(unsigned char *header, unsigned char *body,
                           int headerLen, int bodyLen) = 0;
};

#endif
