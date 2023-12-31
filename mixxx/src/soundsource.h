/***************************************************************************
                          soundsource.h  -  description
                             -------------------
    begin                : Wed Feb 20 2002
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

#ifndef SOUNDSOURCE_H
#define SOUNDSOURCE_H

#include "defs.h"
#include <QString>

#define MIXXX_SOUNDSOURCE_API_VERSION 1
/** @note SoundSource API Version history:
           1 - Mixxx 1.8.0 Beta 2
           ...
  */

/** Getter function to be declared by all SoundSource plugins */
class SoundSource;
typedef SoundSource* (*getSoundSourceFunc)(QString filename);
typedef char** (*getSupportedFileExtensionsFunc)();
typedef int (*getSoundSourceAPIVersionFunc)();


/*
  Base class for sound sources.
*/
class SoundSource
{
public:
    SoundSource(QString qFilename);
    virtual ~SoundSource();
    virtual int open() = 0;
    virtual long seek(long) = 0;
    virtual unsigned read(unsigned long size, const SAMPLE*) = 0;
    virtual long unsigned length() = 0;
    static float str2bpm( QString sBpm );
    virtual int parseHeader() = 0;
    //static QList<QString> supportedFileExtensions(); //CRAP can't do this!
    /** Return a list of cue points stored in the file */
    virtual QList<long> *getCuePoints();
    /** Returns filename */
    virtual QString getFilename();
    /** Return artist name */
    virtual QString getArtist();
    /** Return track title */
    virtual QString getTitle();
    virtual QString getAlbum();
    virtual QString getType();
    virtual QString getComment();
    virtual QString getYear();
    virtual QString getGenre();
    virtual QString getTrackNumber();
    virtual float getBPM();
    virtual int getDuration();
    virtual int getBitrate();
    virtual unsigned int getSampleRate();
    virtual int getChannels();

    virtual void setArtist(QString);
    virtual void setTitle(QString);
    virtual void setAlbum(QString);
    virtual void setType(QString);
    virtual void setComment(QString);
    virtual void setYear(QString);
    virtual void setGenre(QString);
    virtual void setTrackNumber(QString);
    virtual void setBPM(float);
    virtual void setDuration(int);
    virtual void setBitrate(int);
    virtual void setSampleRate(unsigned int);
    virtual void setChannels(int);
protected:
    /** File name */
    QString m_qFilename;

    QString m_sArtist;
    QString m_sTitle;
    QString m_sAlbum;
    QString m_sType;
    QString m_sComment;
    QString m_sYear;
    QString m_sGenre;
    QString m_sTrackNumber;
    float m_fBPM;
    int m_iDuration;
    int m_iBitrate;
    /** Sample rate of the file */
    unsigned int m_iSampleRate;
    int m_iChannels;
    //Dontcha be forgettin' to initialize these variables.... arr

};

#endif
