/***************************************************************************
                          wpixmapstore.cpp  -  description
                             -------------------
    begin                : Mon Jul 28 2003
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

#include "wpixmapstore.h"
//Added by qt3to4:
#include <QPixmap>
#include <QtDebug>

Q3Dict<PixmapInfoType> WPixmapStore::dictionary(251);

ImgSource * WPixmapStore::loader = 0;

WPixmapStore::WPixmapStore()
{
}

QPixmap * WPixmapStore::getPixmap(const QString &fileName)
{
    // Search for pixmap in list
    PixmapInfoType * info;

    info = dictionary[fileName];
    if (info)
    {
        info->instCount++;
        return info->pixmap;
    }

    // Pixmap wasn't found, construct it
//    qDebug() << "Loading pixmap" << fileName;
    info = new PixmapInfoType;
    if (loader != 0) {
        QImage * img = loader->getImage(fileName);

        info->pixmap = new QPixmap(QPixmap::fromImage(*img)); //ack, hacky; there must be a better way (we're using pixmap pointers, but perhaps qt4 expects that you'll just copy?) --kousu 2009/03	
        // No longer need the original QImage (I hope...) - adam_d
        delete img;
    } else {
        info->pixmap = new QPixmap(fileName);
    }
    info->instCount = 1;

    dictionary.insert(fileName, info);

    return info->pixmap;
}

QPixmap * WPixmapStore::getPixmapNoCache(const QString& fileName) {
    if (loader != 0) {
        QImage * img = loader->getImage(fileName);
        QPixmap r = QPixmap::fromImage(*img);
        delete img;
        return new QPixmap(r); //ack, hacky; there must be a better way (we're using pixmap pointers, but perhaps qt4 expects that you'll just copy?) --kousu 2009/03
    } else {
        return new QPixmap(fileName);
    }
}

void WPixmapStore::deletePixmap(QPixmap * p)
{
    // Search for pixmap in list
    Q3DictIterator<PixmapInfoType> it(dictionary);

    for( ; it.current(); ++it )
    {
        PixmapInfoType * info = it.current();

        if (p == info->pixmap)
        {
            info->instCount--;
            if (info->instCount<1)
            {
                delete info->pixmap;
                dictionary.remove(it.currentKey());
                delete info;
            }

            break;
        }
    }
}

void WPixmapStore::emptyStore() {

}

void WPixmapStore::setLoader(ImgSource * ld) {
    loader = ld;
}
