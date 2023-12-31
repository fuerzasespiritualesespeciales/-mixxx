// wbrowsetableview.cpp
// Created 10/19/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "controlobject.h"
#include "widget/wbrowsetableview.h"

WBrowseTableView::WBrowseTableView(QWidget* parent,
                                   ConfigObject<ConfigValue>* pConfig)
        : WLibraryTableView(parent, pConfig,
                            ConfigKey("[Library]", "BrowseVScrollBarPos")),
          m_player1Act(tr("Load in Player 1"), this),
          m_player2Act(tr("Load in Player 2"), this),
          m_contextMenu(this) {
    connect(&m_player1Act, SIGNAL(triggered()),
            this, SLOT(slotLoadPlayer1()));
    connect(&m_player2Act, SIGNAL(triggered()),
            this, SLOT(slotLoadPlayer2()));
    m_contextMenu.addAction(&m_player1Act);
    m_contextMenu.addAction(&m_player2Act);
}

WBrowseTableView::~WBrowseTableView() {

}

void WBrowseTableView::contextMenuEvent(QContextMenuEvent* pEvent) {
    // TODO(XXX) Get rid of this!
    m_player1Act.setEnabled(
        ControlObject::getControl(ConfigKey("[Channel1]","play"))->get()==0.);
    m_player2Act.setEnabled(
        ControlObject::getControl(ConfigKey("[Channel2]","play"))->get()==0.);

    m_contextMenu.exec(pEvent->globalPos());
}

void WBrowseTableView::slotLoadPlayer1() {
    QModelIndexList selectedIndices = selectionModel()->selectedRows();
    if (selectedIndices.size() > 0) {
        emit(loadToPlayer(selectedIndices.at(0), 1));
    }
}

void WBrowseTableView::slotLoadPlayer2() {
    QModelIndexList selectedIndices = selectionModel()->selectedRows();
    if (selectedIndices.size() > 0) {
        emit(loadToPlayer(selectedIndices.at(0), 2));
    }
}

void WBrowseTableView::onSearchStarting() {
    emit(searchStarting());
    saveVScrollBarPos();
}

void WBrowseTableView::onSearchCleared() {
    restoreVScrollBarPos();
    emit(searchCleared());
}

void WBrowseTableView::onSearch(const QString& text) {
    emit(search(text));

}

void WBrowseTableView::onShow()
{
}

QWidget* WBrowseTableView::getWidgetForMIDIControl()
{
    return this;
}

void WBrowseTableView::keyPressEvent(QKeyEvent* event)
{
    QModelIndexList selectedIndices = selectionModel()->selectedRows();
    if (event->key() == Qt::Key_Return)
    {
        if (selectedIndices.size() > 0) {
            QModelIndex index = selectedIndices.at(0);
            emit(activated(index));
        }
    }
    if (event->key() == Qt::Key_BracketLeft)
    {
        slotLoadPlayer1();
    }
    if (event->key() == Qt::Key_BracketRight)
    {
        slotLoadPlayer2();
    }
    else
        QTableView::keyPressEvent(event);
}
