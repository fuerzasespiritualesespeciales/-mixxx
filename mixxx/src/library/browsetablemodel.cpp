
#include <QtCore>
#include <QtSql>
#include "browsetablemodel.h"


BrowseTableModel::BrowseTableModel(QObject* parent)
        : QFileSystemModel(parent),
          TrackModel(QSqlDatabase::database("QSQLITE"),
                     "mixxx.db.model.browse")
{
    QFileSystemModel::setReadOnly(true);
    QFileSystemModel::setFilter(QDir::AllDirs | QDir::AllEntries);
    //QFileSystemModel::setSorting(QDir::DirsFirst | Qir::IgnoreCase);

}

BrowseTableModel::~BrowseTableModel()
{

}

TrackPointer BrowseTableModel::getTrack(const QModelIndex& index) const
{
	//TODO

    return TrackPointer();
}

QString BrowseTableModel::getTrackLocation(const QModelIndex& index) const
{
    return filePath(index);
}

void BrowseTableModel::search(const QString& searchText)
{
	//TODO
}

const QString BrowseTableModel::currentSearch()
{
    return QString();
}

bool BrowseTableModel::isColumnInternal(int) {
    return false;
}

void BrowseTableModel::moveTrack(const QModelIndex&, const QModelIndex&) {

}

QItemDelegate* BrowseTableModel::delegateForColumn(const int) {
    return NULL;
}

void BrowseTableModel::removeTrack(const QModelIndex& index)
{
	//TODO
}

void BrowseTableModel::removeTracks(const QModelIndexList& indices)
{
	//TODO
}

bool BrowseTableModel::addTrack(const QModelIndex& index, QString location)
{
	//TODO
    return false;
}

QMimeData* BrowseTableModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urls;

    //Ok, so the list of indexes we're given contains separates indexes for
    //each column, so even if only one row is selected, we'll have like 7 indexes.
    //We need to only count each row once:
    QList<int> rows;

    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            if (!rows.contains(index.row())) {
                rows.push_back(index.row());
                QUrl url = QUrl::fromLocalFile(getTrackLocation(index));
                if (!url.isValid())
                    qDebug() << "ERROR invalid url\n";
                else {
                    urls.append(url);
                    qDebug() << "Appending URL:" << url;
                }
            }
        }
    }
    mimeData->setUrls(urls);
    return mimeData;
}
