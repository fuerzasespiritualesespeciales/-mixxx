// proxytrackmodel.cpp
// Created 10/22/2009 by RJ Ryan (rryan@mit.edu)

#include <QtCore>
#include <QVariant>

#include "library/proxytrackmodel.h"

ProxyTrackModel::ProxyTrackModel(QAbstractItemModel* pTrackModel,
                                 bool bHandleSearches)
        // ProxyTrackModel proxies settings requests to the composed TrackModel,
        // don't initialize its TrackModel with valid parameters.
        : TrackModel(QSqlDatabase(), ""),
          m_bHandleSearches(bHandleSearches) {
    m_pTrackModel = dynamic_cast<TrackModel*>(pTrackModel);
    Q_ASSERT(m_pTrackModel && pTrackModel);
    setSourceModel(pTrackModel);
}

ProxyTrackModel::~ProxyTrackModel() {
}

TrackPointer ProxyTrackModel::getTrack(const QModelIndex& index) const {
    QModelIndex indexSource = mapToSource(index);
    return m_pTrackModel->getTrack(indexSource);
}

QString ProxyTrackModel::getTrackLocation(const QModelIndex& index) const {
    QModelIndex indexSource = mapToSource(index);
    return m_pTrackModel->getTrackLocation(indexSource);
}

void ProxyTrackModel::search(const QString& searchText) {
    if (m_bHandleSearches) {
        m_currentSearch = searchText;
        setFilterFixedString(searchText);
    } else {
        m_pTrackModel->search(searchText);
    }
}

const QString ProxyTrackModel::currentSearch() {
    if (m_bHandleSearches) {
        return m_currentSearch;
    }
    return m_pTrackModel->currentSearch();
}

bool ProxyTrackModel::isColumnInternal(int column) {
    return m_pTrackModel->isColumnInternal(column);
}

void ProxyTrackModel::removeTrack(const QModelIndex& index) {
    QModelIndex indexSource = mapToSource(index);
    m_pTrackModel->removeTrack(indexSource);
}

void ProxyTrackModel::removeTracks(const QModelIndexList& indices) {
    QModelIndexList translatedList;
    foreach (QModelIndex index, indices) {
        QModelIndex indexSource = mapToSource(index);
        translatedList.append(indexSource);
    }
    m_pTrackModel->removeTracks(translatedList);
}

bool ProxyTrackModel::addTrack(const QModelIndex& index, QString location) {
    QModelIndex indexSource = mapToSource(index);
    return m_pTrackModel->addTrack(indexSource, location);
}

void ProxyTrackModel::moveTrack(const QModelIndex& sourceIndex,
                                const QModelIndex& destIndex) {
    QModelIndex sourceIndexSource = mapToSource(sourceIndex);
    QModelIndex destIndexSource = mapToSource(destIndex);
    m_pTrackModel->moveTrack(sourceIndexSource, destIndexSource);
}

QItemDelegate* ProxyTrackModel::delegateForColumn(const int i) {
    return m_pTrackModel->delegateForColumn(i);
}

TrackModel::CapabilitiesFlags ProxyTrackModel::getCapabilities() const {
    return m_pTrackModel->getCapabilities();
}

bool ProxyTrackModel::filterAcceptsRow(int sourceRow,
                                       const QModelIndex& sourceParent) const {

    if (!m_bHandleSearches)
        return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);

    const QList<int>& filterColumns = m_pTrackModel->searchColumns();
    QAbstractItemModel* itemModel =
            dynamic_cast<QAbstractItemModel*>(m_pTrackModel);
    bool rowMatches = false;

    QRegExp filter = filterRegExp();
    QListIterator<int> iter(filterColumns);

    while (!rowMatches && iter.hasNext()) {
        int i = iter.next();
        QModelIndex index = itemModel->index(sourceRow, i, sourceParent);
        QVariant data = itemModel->data(index);
        if (qVariantCanConvert<QString>(data)) {
            QString strData = qVariantValue<QString>(data);
            if (strData.contains(filter))
                rowMatches = true;
        }
    }

    return rowMatches;
}

QString ProxyTrackModel::getModelSetting(QString name) {
    if (!m_pTrackModel)
        return QString();
    return m_pTrackModel->getModelSetting(name);
}

bool ProxyTrackModel::setModelSetting(QString name, QVariant value) {
    if (!m_pTrackModel)
        return false;
    return m_pTrackModel->setModelSetting(name, value);
}
