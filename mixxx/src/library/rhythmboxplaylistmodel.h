/***************************************************************************
                           rhythmboxPlaylistsource.h
                              -------------------
     begin                : 8/17/2009
     copyright            : (C) 2009 Phillip Whelan
     email                : pwhelan@mixxx.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RHYTHMBOXPLAYLISTMODEL_H
#define RHYTHMBOXPLAYLISTMODEL_H

#include <QtSql>
#include <QtXml>
#include <QList>

#include "trackmodel.h"
#include "rhythmboxtrackmodel.h"


#define MIXXX_RHYTHMBOX_DB_LOCATION QDir::homePath() + "/.gnome2/rhythmbox/playlists.xml"
#define MIXXX_RHYTHMBOX_DB_LOCATION_ALT QDir::homePath() + "/.local/share/rhythmbox/playlists.xml"

/**
   @author Phillip Whelan
   Code copied originally from RhythmBoxTrackModel
*/
class RhythmboxPlaylistModel : public QAbstractTableModel, public TrackModel
{
    enum Columns {
        COLUMN_ARTIST = 0,
        COLUMN_TITLE,
        COLUMN_ALBUM,
        COLUMN_DATE,
        COLUMN_GENRE,
        COLUMN_LOCATION,
        COLUMN_DURATION,
        NUM_COLUMNS
    };

    Q_OBJECT
    public:
    RhythmboxPlaylistModel(RhythmboxTrackModel*);
    virtual ~RhythmboxPlaylistModel();

    //QAbstractTableModel stuff
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    QMimeData* mimeData(const QModelIndexList &indexes) const;
    virtual QVariant data(const QModelIndex & index,
                          int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section,
                                Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent) const;

    //Playlist Model stuff
    virtual TrackPointer getTrack(const QModelIndex& index) const;
    virtual QString getTrackLocation(const QModelIndex& index) const;
    virtual void search(const QString& searchText);
    virtual const QString currentSearch();
    virtual const QList<int>& searchColumns() const;
    virtual bool isColumnInternal(int column);
    virtual void removeTrack(const QModelIndex& index);
    virtual void removeTracks(const QModelIndexList& indices);
    virtual bool addTrack(const QModelIndex& index, QString location);
    virtual void moveTrack(const QModelIndex& sourceIndex,
                           const QModelIndex& destIndex);
    QItemDelegate* delegateForColumn(const int i);

    virtual QList<QString> getPlaylists();
    virtual void setPlaylist(QString playlist);

    int numPlaylists();
    QString playlistTitle(int n);


/*
 	void scanPath(QString path);
 	bool trackExistsInDatabase(QString file_location);

 	QList<TrackInfoObject*> dumpDB();

 	QSqlDatabase getDatabase();
 */
  public slots:


  signals:
    void startedLoading();
    void progressLoading(QString path);
    void finishedLoading();

private:
    QDomNodeList m_playlistNodes;
    RhythmboxTrackModel *m_pRhythmbox;
    QMap<QString, QDomNodeList> m_mPlaylists;
    QString m_sCurrentPlaylist;
    QString m_currentSearch;
};

#endif
