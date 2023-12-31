// rhythmboxfeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef RHYTHMBOXFEATURE_H
#define RHYTHMBOXFEATURE_H

#include <QStringListModel>

#include "library/libraryfeature.h"

class RhythmboxPlaylistModel;
class RhythmboxTrackModel;
class ProxyTrackModel;

class RhythmboxFeature : public LibraryFeature {
 Q_OBJECT
 public:
    RhythmboxFeature(QObject* parent = NULL);
    virtual ~RhythmboxFeature();
    static bool isSupported();

    QVariant title();
    QIcon getIcon();

    bool dropAccept(QUrl url);
    bool dropAcceptChild(const QModelIndex& index, QUrl url);
    bool dragMoveAccept(QUrl url);
    bool dragMoveAcceptChild(const QModelIndex& index, QUrl url);

    QAbstractItemModel* getChildModel();

public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void onRightClick(const QPoint& globalPos);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);

private:
    RhythmboxTrackModel* m_pRhythmboxTrackModel;
    RhythmboxPlaylistModel* m_pRhythmboxPlaylistModel;
    ProxyTrackModel* m_pTrackModelProxy;
    ProxyTrackModel* m_pPlaylistModelProxy;
    QStringListModel m_childModel;
};

#endif /* RHYTHMBOXFEATURE_H */
