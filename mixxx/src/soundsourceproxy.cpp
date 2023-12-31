/* -*- mode:C++; indent-tabs-mode:t; tab-width:8; c-basic-offset:4; -*- */
/***************************************************************************
                          soundsourceproxy.cpp  -  description
                             -------------------
    begin                : Wed Oct 13 2004
    copyright            : (C) 2004 Tue Haste Andersen
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

#include "trackinfoobject.h"
#include "soundsourceproxy.h"
#include "soundsourcemp3.h"
#include "soundsourceoggvorbis.h"
#ifdef __SNDFILE__
#include "soundsourcesndfile.h"
#endif
#ifdef __FFMPEGFILE__
#include "soundsourceffmpeg.h"
#endif

#include <QLibrary>
#include <QMutexLocker>
#include <QMutex>
#include <QDebug>
#include <QDir>
#include <QDesktopServices>
#include <QCoreApplication>
#include <QApplication>


//Static memory allocation
QMap<QString, QLibrary*> SoundSourceProxy::m_plugins;
QMap<QString, getSoundSourceFunc> SoundSourceProxy::m_extensionsSupportedByPlugins;
QMutex SoundSourceProxy::m_extensionsMutex;


//Constructor
SoundSourceProxy::SoundSourceProxy(QString qFilename)
	: SoundSource(qFilename),
	  m_pSoundSource(NULL),
	  m_pTrack() {
    m_pSoundSource = initialize(qFilename);
}

//Other constructor
SoundSourceProxy::SoundSourceProxy(TrackPointer pTrack)
	: SoundSource(pTrack->getLocation()),
	  m_pSoundSource(NULL) {

    m_pSoundSource = initialize(pTrack->getLocation());
    m_pTrack = pTrack;
}

void SoundSourceProxy::loadPlugins()
{
    /** Scan for and initialize all plugins */

    QList<QDir> pluginDirs;
    QStringList nameFilters;

    QStringList clArgs = QApplication::arguments();
    int pluginPath = clArgs.indexOf("--pluginPath");
    if (pluginPath != -1 && pluginPath + 1 < clArgs.size()) {
        qDebug() << "Adding plugin path from commandline arg:" << clArgs.at(pluginPath + 1);
        pluginDirs.append(QDir(clArgs.at(pluginPath + 1)));
    }
#ifdef __LINUX__
    pluginDirs.append(QDir("/usr/local/lib/mixxx/plugins/soundsource/"));
    pluginDirs.append(QDir("/usr/lib/mixxx/plugins/soundsource/"));
    pluginDirs.append(QDir(QDesktopServices::storageLocation(QDesktopServices::HomeLocation) + "/.mixxx/plugins/soundsource/"));
#elif __WINDOWS__
    pluginDirs.append(QDir(QCoreApplication::applicationDirPath() + "/plugins/soundsource/"));
#elif __APPLE__
    QString bundlePluginDir = QCoreApplication::applicationDirPath(); //blah/Mixxx.app/Contents/MacOS
    bundlePluginDir.remove("MacOS");
    //blah/Mixxx.app/Contents/PlugIns/soundsource
    //bundlePluginDir.append("PlugIns/soundsource");  //Our SCons bundle target doesn't handle plugin subdirectories :(
    bundlePluginDir.append("PlugIns/");
    pluginDirs.append(QDir(bundlePluginDir));
    pluginDirs.append(QDir("/Library/Application Support/Mixxx/Plugins/soundsource/"));
    nameFilters << "libsoundsource*";
#endif

    QDir dir;
    foreach(dir, pluginDirs)
    {
        QStringList files = dir.entryList(nameFilters, QDir::Files | QDir::NoDotAndDotDot);
        QString file;
        foreach (file, files)
        {
            getPlugin(dir.filePath(file));
        }
    }
}

SoundSource* SoundSourceProxy::initialize(QString qFilename) {

    SoundSource* sndsrc = NULL;
    QString extension = qFilename;
    extension.remove(0, (qFilename.lastIndexOf(".")+1));
    extension = extension.toLower();

#ifdef __FFMPEGFILE__
    return new SoundSourceFFmpeg(qFilename);
#endif

    if (SoundSourceMp3::supportedFileExtensions().contains(extension)) {
	    return new SoundSourceMp3(qFilename);
    } else if (SoundSourceOggVorbis::supportedFileExtensions().contains(extension)) {
	    return new SoundSourceOggVorbis(qFilename);
    } else if (m_extensionsSupportedByPlugins.contains(extension)) {
        getSoundSourceFunc getter = m_extensionsSupportedByPlugins.value(extension);
        if (getter)
        {
            qDebug() << "Getting SoundSource plugin object for" << extension;
            return getter(qFilename);
        }
        else {
            qDebug() << "Failed to resolve getSoundSource in plugin for" <<
                        extension;
            return NULL; //Failed to load plugin
        }
#ifdef __SNDFILE__
    } else if (SoundSourceSndFile::supportedFileExtensions().contains(extension)) {
	    return new SoundSourceSndFile(qFilename);
#endif
    } else { //Unsupported filetype
        return NULL;
    }
}

SoundSourceProxy::~SoundSourceProxy()
{
    delete m_pSoundSource;
}

QLibrary* SoundSourceProxy::getPlugin(QString lib_filename)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    QLibrary* plugin;
    if (m_plugins.contains(lib_filename))
    	plugin = m_plugins.value(lib_filename);
    else {
    	plugin = new QLibrary(lib_filename);
        if (!plugin->load()) {
            qDebug() << "Failed to dynamically load" << lib_filename << plugin->errorString();
        } else {
            qDebug() << "Dynamically loaded" << lib_filename;
            //Add the plugin to our list of loaded QLibraries/plugins
            m_plugins.insert(lib_filename, plugin);

            bool incompatible = false;
            //Plugin API version check
            getSoundSourceAPIVersionFunc getver = (getSoundSourceAPIVersionFunc)plugin->resolve("getSoundSourceAPIVersion");
            if (getver) {
                int pluginAPIVersion = getver();
                if (pluginAPIVersion != MIXXX_SOUNDSOURCE_API_VERSION) {
                    //SoundSource API version mismatch
                    incompatible = true;
                }
            } else {
                //Missing getSoundSourceAPIVersion symbol 
                incompatible = true;
            }
            if (incompatible)
            {
                //Plugin is using an older/incompatible version of the
                //plugin API!
                qDebug() << "Plugin" << lib_filename << "is incompatible with your version of Mixxx!";
                return NULL;
            }

            //Map the file extensions this plugin supports onto a function
            //pointer to the "getter" function that gets a SoundSourceBlah.
            getSoundSourceFunc getter = (getSoundSourceFunc)plugin->resolve("getSoundSource");
            Q_ASSERT(getter); //Getter function not found.
                              //Did you export it properly in your plugin?
            getSupportedFileExtensionsFunc getFileExts = (getSupportedFileExtensionsFunc)plugin->resolve("supportedFileExtensions");
            Q_ASSERT(getFileExts);
            char** supportedFileExtensions = getFileExts();
            int i = 0;
            while (supportedFileExtensions[i] != NULL)
            {
                qDebug() << "Plugin supports:" << supportedFileExtensions[i];
                m_extensionsSupportedByPlugins.insert(QString(supportedFileExtensions[i]), getter);
                free(supportedFileExtensions[i]);
                i++;
            }
            free(supportedFileExtensions);
            //So now we have a list of file extensions (eg. "m4a", "mp4", etc)
            //that map onto the getter function for this plugin (eg. the
            //function that returns a SoundSourceM4A object)
        }
    }
    return plugin;
}


int SoundSourceProxy::open()
{
    if (!m_pSoundSource) {
        return 0;
    }
    int retVal = m_pSoundSource->open();

    //Update some metadata (currently only the duration)
    //after a song is open()'d. Eg. We don't know the length
    //of VBR MP3s until we've seeked through and counted all
    //the frames. We don't do that in ParseHeader() to keep
    //library scanning fast.
    // .... but only do this if the song doesn't already
    //      have a duration parsed. (Some SoundSources don't
    //      parse metadata on open(), so they won't have the
    //      duration.)
    // SSMP3 will set duration to -1 on VBR files,
    //  so we must look for that here too
    if (m_pTrack->getDuration() <= 0)
        m_pTrack->setDuration(m_pSoundSource->getDuration());

    return retVal;
}

long SoundSourceProxy::seek(long l)
{
    if (!m_pSoundSource) {
	return 0;
    }
    return m_pSoundSource->seek(l);
}

unsigned SoundSourceProxy::read(unsigned long size, const SAMPLE * p)
{
    if (!m_pSoundSource) {
	return 0;
    }
    return m_pSoundSource->read(size, p);
}

long unsigned SoundSourceProxy::length()
{
    if (!m_pSoundSource) {
	return 0;
    }
    return m_pSoundSource->length();
}

int SoundSourceProxy::parseHeader()
{
    //TODO: Reorganize code so that the static ParseHeader isn't needed, and use this function instead?
    return 0;
}

int SoundSourceProxy::ParseHeader(TrackInfoObject* p)
{

    QString qFilename = p->getLocation();
    SoundSource* sndsrc = initialize(qFilename);
    if (sndsrc == NULL)
        return ERR;

    if (sndsrc->parseHeader() == OK) {
        //Dump the metadata from the soundsource into the TIO
        //qDebug() << "Album:" << sndsrc->getAlbum(); //Sanity check to make sure we've actually parsed metadata and not the filename
        p->setArtist(sndsrc->getArtist());
        QString title = sndsrc->getTitle();
        if (title.isEmpty()) {
            // If no title is returned, use the file name (without the extension)
            int start = qFilename.lastIndexOf(QRegExp("[/\\\\]"))+1;
            int end = qFilename.lastIndexOf('.');
            if (end == -1) end = qFilename.length();
            title = qFilename.mid(start,end-start);
        }
        p->setTitle(title);
        p->setAlbum(sndsrc->getAlbum());
        p->setType(sndsrc->getType());
        p->setYear(sndsrc->getYear());
        p->setGenre(sndsrc->getGenre());
	p->setComment(sndsrc->getComment());
        p->setTrackNumber(sndsrc->getTrackNumber());
        p->setBpm(sndsrc->getBPM());
        p->setDuration(sndsrc->getDuration());
        p->setBitrate(sndsrc->getBitrate());
        p->setSampleRate(sndsrc->getSampleRate());
        p->setChannels(sndsrc->getChannels());
        p->setHeaderParsed(true);
    }
    else
    {
        p->setHeaderParsed(false);
    }
    delete sndsrc;

    return 0;
}

QList<QString> SoundSourceProxy::supportedFileExtensions()
{
    QMutexLocker locker(&m_extensionsMutex);
    QList<QString> supportedFileExtensions;
    supportedFileExtensions.append(SoundSourceMp3::supportedFileExtensions());
    supportedFileExtensions.append(SoundSourceOggVorbis::supportedFileExtensions());
    supportedFileExtensions.append(SoundSourceSndFile::supportedFileExtensions());
    supportedFileExtensions.append(m_extensionsSupportedByPlugins.keys());

    return supportedFileExtensions;
}

QString SoundSourceProxy::supportedFileExtensionsString()
{
    QList<QString> supportedFileExtList = SoundSourceProxy::supportedFileExtensions();
    QString supportedFileExtString;

    //Turn the list into a "*.mp3 *.wav *.etc" style string
    QString ext;
    QListIterator<QString> it(supportedFileExtList);
    while (it.hasNext())
    {
        ext = it.next();
        supportedFileExtString.append(QString("*.%1").arg(ext));
        if (it.hasNext())
            supportedFileExtString.append(" ");
    }

    return supportedFileExtString;
}

QString SoundSourceProxy::supportedFileExtensionsRegex()
{
    QList<QString> supportedFileExtList = SoundSourceProxy::supportedFileExtensions();
    QString supportedFileExtRegex = "\\.(";

    //Turn the list into a "\\.(mp3|wav|etc)" style regex string
    QString ext;
    QListIterator<QString> it(supportedFileExtList);
    while (it.hasNext())
    {
        ext = it.next();
        supportedFileExtRegex.append(QString("%1").arg(ext));
        if (it.hasNext())
            supportedFileExtRegex.append("|");
    }
    supportedFileExtRegex.append(")");

    return supportedFileExtRegex;
}

unsigned int SoundSourceProxy::getSampleRate()
{
    if (!m_pSoundSource) {
	return 0;
    }
    return m_pSoundSource->getSampleRate();
}


QString SoundSourceProxy::getFilename()
{
    if (!m_pSoundSource) {
	return "";
    }
    return m_pSoundSource->getFilename();
}
