/***************************************************************************
                          wpixmapstore.h  -  description
                             -------------------
    begin                : Mon Jun 28 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WPIXMAPSTORE_H
#define WPIXMAPSTORE_H

#include <qpixmap.h>
#include <q3dict.h>

#include "imgsource.h"

/**
  *
  *@author Tue & Ken Haste Andersen
  */

typedef struct
{
    QPixmap *pixmap;
    int instCount;
} PixmapInfoType;

class WPixmapStore {
public:
    WPixmapStore();
    static QPixmap *getPixmap(const QString &fileName);
	static QPixmap *getPixmapNoCache(const QString &fileName);
    static void deletePixmap(QPixmap *p);
	static void setLoader(ImgSource* ld);
	static void emptyStore();
private:
    /** Dictionary of pixmaps already instantiated */
    static Q3Dict<PixmapInfoType> dictionary;
	static ImgSource* loader;
};

#endif
