/***************************************************************************
                          libraryscanner.cpp  -  scans library in a thread
                             -------------------
    begin                : 11/27/2007
    copyright            : (C) 2007 Albert Santoni
    email                : gamegod \a\t users.sf.net
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtCore>
#include <QtDebug>
#include <QDesktopServices>

#include "soundsourceproxy.h"
#include "library/legacylibraryimporter.h"
#include "libraryscanner.h"
#include "libraryscannerdlg.h"
#include "trackinfoobject.h"

LibraryScanner::LibraryScanner(TrackCollection* collection) :
    m_pCollection(collection),
    m_libraryHashDao(m_database),
    m_cueDao(m_database),
    m_trackDao(m_database, m_cueDao),
    m_playlistDao(m_database),
    //Don't initialize m_database here, we need to do it in run() so the DB conn is in
    //the right thread.
    nameFilters(SoundSourceProxy::supportedFileExtensionsString().split(" "))
{

    qDebug() << "Constructed LibraryScanner!!!";
    resetCancel();

    //Force the GUI thread's TrackInfoObject cache to be cleared
    //when a library scan is finished, because we might have
    //modified the database directly when we detected moved files,
    //and the TIOs corresponding to the moved files would then have the
    //wrong track location.
    connect(this, SIGNAL(scanFinished()),
            &(collection->getTrackDAO()), SLOT(clearCache()));

    /* The "Album Artwork" folder within iTunes stores Album Arts.
     * It has numerous hundreds of sub folders but no audio files
     * We put this folder on a "black list"
     * On Windows, the iTunes folder is contained within the standard music folder
     * Hence, Mixxx will scan the "Album Arts folder" for standard users which is wasting time
     */
    QString iTunesArtFolder = "";
#if defined(__WINDOWS__)
		iTunesArtFolder = QDesktopServices::storageLocation(QDesktopServices::MusicLocation) + "\\iTunes\\Album Artwork";
		iTunesArtFolder.replace(QString("\\"), QString("/"));
#elif defined(__APPLE__)
		iTunesArtFolder = QDesktopServices::storageLocation(QDesktopServices::MusicLocation) + "/iTunes/Album Artwork";
#endif
    m_directoriesBlacklist << iTunesArtFolder;
    qDebug() << "iTunes Album Art path is:" << iTunesArtFolder;

#ifdef __WINDOWS__
    //Blacklist the _Serato_ directory that pollutes "My Music" on Windows.
    QString seratoDir = QDesktopServices::storageLocation(QDesktopServices::MusicLocation) + "\\_Serato_";
    m_directoriesBlacklist << seratoDir;
#endif
}

LibraryScanner::~LibraryScanner()
{
    //IMPORTANT NOTE: This code runs in the GUI thread, so it should _NOT_ use
    //                the m_trackDao that lives inside this class. It should use
    //                the DAOs that live in m_pTrackCollection.

    if (isRunning()) {
        //Cancel any running library scan...
        m_pCollection->slotCancelLibraryScan();
        this->cancel();

        wait(); //Wait for thread to finish
    }

    //Do housekeeping on the LibraryHashes table.
    m_pCollection->getDatabase().transaction();

    //Mark the corresponding file locations in the track_locations table as deleted
    //if we find one or more deleted directories.
    QStringList deletedDirs;
    QSqlQuery query(m_pCollection->getDatabase());
    query.prepare("SELECT directory_path FROM LibraryHashes "
                  "WHERE directory_deleted=1");
    if (query.exec()) {
        while (query.next()) {
            QString directory = query.value(query.record().indexOf("directory_path")).toString();
            deletedDirs << directory;
        }
    } else {
        qDebug() << "Couldn't SELECT deleted directories" << query.lastError();
    }

    //Delete any directories that have been marked as deleted...
    query.finish();
    query.exec("DELETE FROM LibraryHashes "
               "WHERE directory_deleted=1");

    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << query.lastError();
    }

    QString dir;
    foreach(dir, deletedDirs) {
        m_pCollection->getTrackDAO().markTrackLocationsAsDeleted(dir);
    }

    m_pCollection->getDatabase().commit();

    //Close our database connection
    Q_ASSERT(!m_database.rollback()); //Rollback any uncommitted transaction
    //The above is an ASSERT because there should never be an outstanding
    //transaction when this code is called. If there is, it means we probably
    //aren't committing a transaction somewhere that should be.
    if (m_database.isOpen())
        m_database.close();

    qDebug() << "LibraryScanner destroyed";
}

void LibraryScanner::run()
{
    unsigned static id = 0; //the id of this thread, for debugging purposes //XXX copypasta (should factor this out somehow), -kousu 2/2009
    QThread::currentThread()->setObjectName(QString("LibraryScanner %1").arg(++id));
    //m_pProgress->slotStartTiming();

    //Lower our priority to help not grind crappy computers.
    setPriority(QThread::LowPriority);

    if (!m_database.isOpen()) {
        m_database = QSqlDatabase::addDatabase("QSQLITE", "LIBRARY_SCANNER");
        m_database.setHostName("localhost");
        m_database.setDatabaseName(MIXXX_DB_PATH);
        m_database.setUserName("mixxx");
        m_database.setPassword("mixxx");

        //Open the database connection in this thread.
        if (!m_database.open()) {
            qDebug() << "Failed to open database from library scanner thread." << m_database.lastError();
            return;
        }
    }

    m_libraryHashDao.setDatabase(m_database);
    m_cueDao.setDatabase(m_database);
    m_trackDao.setDatabase(m_database);
    m_playlistDao.setDatabase(m_database);

    m_libraryHashDao.initialize();
    m_cueDao.initialize();
    m_trackDao.initialize();
    m_playlistDao.initialize();

    m_pCollection->resetLibaryCancellation();

    QTime t2;
    t2.start();
    //Try to upgrade the library from 1.7 (XML) to 1.8+ (DB) if needed
    LegacyLibraryImporter libImport(m_trackDao, m_playlistDao);
    connect(&libImport, SIGNAL(progress(QString)),
            m_pProgress, SLOT(slotUpdate(QString)),
            Qt::BlockingQueuedConnection);
    m_database.transaction();
    libImport.import();
    m_database.commit();
    qDebug("Legacy importer took %d ms", t2.elapsed());

    //Refresh the name filters in case we loaded new
    //SoundSource plugins.
    nameFilters = SoundSourceProxy::supportedFileExtensionsString().split(" ");

    // Time the library scanner.
    QTime t;
    t.start();

    //First, we're going to mark all the directories that we've
    //previously hashed as needing verification. As we search through the directory tree
    //when we rescan, we'll mark any directory that does still exist as verified.
    m_libraryHashDao.invalidateAllDirectories();

    //Mark all the tracks in the library as needing
    //verification of their existance...
    //(ie. we want to check they're still on your hard drive where
    //we think they are)
    m_trackDao.invalidateTrackLocationsInLibrary(m_qLibraryPath);

    qDebug() << "Recursively scanning library.";
    //Start scanning the library.
    //THIS SHOULD NOT BE IN A TRANSACTION! Each addTrack() call from inside
    //recursiveScan() handles it's own transactions.

    QList<TrackInfoObject*> tracksToAdd;

    bool bScanFinishedCleanly = recursiveScan(m_qLibraryPath, tracksToAdd);

    if (!bScanFinishedCleanly) {
        qDebug() << "Recursive scan interrupted.";
    } else {
        qDebug() << "Recursive scan finished cleanly.";
    }
	/*
     * We store the scanned files in the database: Note that the recursiveScan()
     * method used TrackCollection::importDirectory() to scan the folders. The
     * method TrackCollection::importDirectory() added all the files to the
     * 'tracksToAdd' list.
     *
     * The following statement writes all the scanned tracks in the list to the
     * database at once. We don't care if the scan has been cancelled or not.
     *
     * This new approach accelerates the scanning process massively by a factor
     * of 3-4 !!!
     */

    // Runs inside a transaction
    m_trackDao.addTracks(tracksToAdd);

    QMutableListIterator<TrackInfoObject*> it(tracksToAdd);
    while (it.hasNext()) {
        TrackInfoObject* pTrack = it.next();
        it.remove();
        delete pTrack;
    }

    //Start a transaction for all the library hashing (moved file detection)
    //stuff.
    m_database.transaction();

    //At the end of a scan, mark all tracks and directories that
    //weren't "verified" as "deleted" (as long as the scan wasn't cancelled
    //half way through. This condition is important because our rescanning
    //algorithm starts by marking all tracks and dirs as unverified, so a
    //cancelled scan might leave half of your library as unverified. Don't
    //want to mark those tracks/dirs as deleted in that case) :)
    if (bScanFinishedCleanly)
    {
        qDebug() << "Marking unverified tracks as deleted.";
        m_trackDao.markUnverifiedTracksAsDeleted();
        qDebug() << "Marking unverified directories as deleted.";
        m_libraryHashDao.markUnverifiedDirectoriesAsDeleted();

        //Check to see if the "deleted" tracks showed up in another location,
        //and if so, do some magic to update all our tables.
        qDebug() << "Detecting moved files.";
        m_trackDao.detectMovedFiles();

        //Remove the hashes for any directories that have been
        //marked as deleted to clean up. We need to do this otherwise
        //we can skip over songs if you move a set of songs from directory
        //A to B, then back to A.
        m_libraryHashDao.removeDeletedDirectoryHashes();

        m_database.commit();
        qDebug() << "Scan finished cleanly";
    }
    else {
        m_database.rollback();
        qDebug() << "Scan cancelled";
    }

    qDebug("Scan took: %d ms", t.elapsed());

    //m_pProgress->slotStopTiming();

    Q_ASSERT(!m_database.rollback()); //Rollback any uncommitted transaction
    //The above is an ASSERT because there should never be an outstanding
    //transaction when this code is called. If there is, it means we probably
    //aren't committing a transaction somewhere that should be.
    m_database.close();

    resetCancel();
    emit(scanFinished());
}

void LibraryScanner::scan(QString libraryPath)
{
    m_qLibraryPath = libraryPath;
    m_pProgress = new LibraryScannerDlg();

    //The important part here is that we need to use
    //Qt::BlockingQueuedConnection, because we're sending these signals across
    //threads. Normally you'd use regular QueuedConnections for this, but since
    //we don't have an event loop running and we need the signals to get
    //processed immediately, we have to use
    //BlockingQueuedConnection. (DirectConnection isn't an option for sending
    //signals across threads.)
    connect(m_pCollection, SIGNAL(progressLoading(QString)),
            m_pProgress, SLOT(slotUpdate(QString)));
            //Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(progressHashing(QString)),
            m_pProgress, SLOT(slotUpdate(QString)));
            //Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(scanFinished()),
            m_pProgress, SLOT(slotScanFinished()));
    connect(m_pProgress, SIGNAL(scanCancelled()),
            m_pCollection, SLOT(slotCancelLibraryScan()));
    connect(m_pProgress, SIGNAL(scanCancelled()),
            this, SLOT(cancel()));
    scan();
}

void LibraryScanner::cancel()
{
    m_libraryScanMutex.lock();
    m_bCancelLibraryScan = 1;
    m_libraryScanMutex.unlock();
}

void LibraryScanner::resetCancel()
{
    m_libraryScanMutex.lock();
    m_bCancelLibraryScan = 0;
    m_libraryScanMutex.unlock();
}

void LibraryScanner::scan()
{
    start(); //Starts the thread by calling run()
}

/** Recursively scan a music library. Doesn't import tracks for any directories that
    have already been scanned and have not changed. Changes are tracked by performing
    a hash of the directory's file list, and those hashes are stored in the database.
*/
bool LibraryScanner::recursiveScan(QString dirPath, QList<TrackInfoObject*>& tracksToAdd)
{
    QDirIterator fileIt(dirPath, nameFilters, QDir::Files | QDir::NoDotAndDotDot);
    QString currentFile;
    bool bScanFinishedCleanly = true;

    //qDebug() << "Scanning dir:" << dirPath;

    QString newHashStr;
    bool prevHashExists = false;
    int newHash = -1;
    int prevHash = -1;
    //Note: A hash of "0" is a real hash if the directory contains no files!

    while (fileIt.hasNext())
    {
	    currentFile = fileIt.next();
	    //qDebug() << currentFile;
	    newHashStr += currentFile;
    }

    //Calculate a hash of the directory's file list.
    newHash = qHash(newHashStr);

    //Try to retrieve a hash from the last time that directory was scanned.
    prevHash = m_libraryHashDao.getDirectoryHash(dirPath);
    prevHashExists = (prevHash == -1) ? false : true;

    //Compare the hashes, and if they don't match, rescan the files in that directory!
    if (prevHash != newHash)
    {
        //If we didn't know about this directory before...
        if (!prevHashExists) {
            m_libraryHashDao.saveDirectoryHash(dirPath, newHash);
        }
        else //Contents of a known directory have changed.
             //Just need to update the old hash in the database and then rescan it.
        {
            qDebug() << "old hash was" << prevHash << "and new hash is" << newHash;
            m_libraryHashDao.updateDirectoryHash(dirPath, newHash, 0);
        }

        //Rescan that mofo!
        bScanFinishedCleanly = m_pCollection->importDirectory(dirPath, m_trackDao, tracksToAdd);
    }
    else //prevHash == newHash
    {
        //The files in the directory haven't changed, so we don't need to do anything, right?
        //Wrong! We need to mark the directory in the database as "existing", so that we can
        //keep track of directories that have been deleted to stop the database from keeping
        //rows about deleted directories around. :)
        //qDebug() << "prevHash == newHash";

        // Mark the directory as verified and not deleted.
        // m_libraryHashDao.markAsExisting(dirPath);
        // m_libraryHashDao.markAsVerified(dirPath);
        m_libraryHashDao.updateDirectoryStatus(dirPath, false, true);

        //We also need to mark the tracks _inside_ this directory as verified.
        //Note that this doesn't mark the tracks as existing, just that they're in
        //the same state as the last time we scanned this directory.
        m_trackDao.markTracksInDirectoryAsVerified(dirPath);

        emit(progressHashing(dirPath));
    }

    //Let us break out of library directory hashing (the actual file scanning
    //stuff is in TrackCollection::importDirectory)
    m_libraryScanMutex.lock();
    bool cancel = m_bCancelLibraryScan;
    m_libraryScanMutex.unlock();
    if (cancel)
        return false;


    //Look at all the subdirectories and scan them recursively...
    QDirIterator dirIt(dirPath, QDir::Dirs | QDir::NoDotAndDotDot);
    while (dirIt.hasNext() && bScanFinishedCleanly)
    {
        QString nextPath = dirIt.next();

        // Skip the iTunes Album Art Folder since it is probably a waste of
        // time.
        if (m_directoriesBlacklist.contains(nextPath))
            continue;

        if (!recursiveScan(nextPath, tracksToAdd))
            bScanFinishedCleanly = false;
    }

    return bScanFinishedCleanly;
}

/**
   Table: LibraryHashes
   PRIMARY KEY string directory
   string hash


   Recursive Algorithm:
   1) QDirIterator, iterate over all _files_ in a directory to construct a giant string.
   2) newHash = Hash that string.
   3) prevHash = SELECT from LibraryHashes * WHERE directory == strDirectory
   4) if (prevHash != newHash) scanDirectory(strDirectory); //Do a NON-RECURSIVE scan of the files in that dir.
   5) For each directory in strDirectory, execute this algorithm.

  */

