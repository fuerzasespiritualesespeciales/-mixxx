//
// C++ Interface: woverview
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef WOVERVIEW_H
#define WOVERVIEW_H

#include <QPaintEvent>
#include <QMouseEvent>
#include <Q3MemArray>
#include <QPixmap>
#include <QColor>
#include <QList>
#include <q3valuelist.h>

#include "trackinfoobject.h"
#include "widget/wwidget.h"


/**
Waveform overview display

@author Tue Haste Andersen
*/

class ControlObject;

class WOverview : public WWidget
{
    Q_OBJECT
  public:
    WOverview(const char* pGroup, QWidget *parent=NULL);
    virtual ~WOverview();
    void setup(QDomNode node);
    void setData(const QByteArray *pWaveformSummary, long liSampleDuration);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *);
    void redrawPixmap();
    QColor getMarkerColor();
    QColor getSignalColor();
  public slots:
    void setValue(double);
    void slotLoadNewWaveform(TrackInfoObject* pTrack);
    void slotLoadNewWaveform(TrackPointer pTrack);
    void slotUnloadTrack(TrackPointer pTrack);
  private slots:
    void cueChanged(double v);
    void loopStartChanged(double v);
    void loopEndChanged(double v);
    void loopEnabledChanged(double v);
  private:

    const char* m_pGroup;
    bool waveformChanged;

    // Loop controls and values
    ControlObject* m_pLoopStart;
    ControlObject* m_pLoopEnd;
    ControlObject* m_pLoopEnabled;
    double m_dLoopStart, m_dLoopEnd;
    bool m_bLoopEnabled;

    // Hotcue controls and values
    QList<ControlObject*> m_hotcueControls;
    QMap<QObject*, int> m_hotcueMap;
    QList<int> m_hotcues;

    /** Array containing waveform summary */
    QByteArray m_waveformSummary;
    /** Duration of current track in samples */
    int m_liSampleDuration;
    /** True if slider is dragged. Only used when m_bEventWhileDrag is false */
    bool m_bDrag;
    /** Internal storage of slider position in pixels */
    int m_iPos, m_iStartMousePos;
    /** Pointer to screen buffer */
    QPixmap *m_pScreenBuffer;
    QColor m_qColorMarker;
    QColor m_qColorSignal;
};

#endif
