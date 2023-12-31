/***************************************************************************
                          mixxxkeyboard.h  -  description
                             -------------------
    begin                : Wed Dec 2 2003
    copyright            : (C) 2003 by Tue Haste Andersen
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

#ifndef MIXXXKEYBOARD_H
#define MIXXXKEYBOARD_H

#include <qobject.h>
#include <qevent.h>
#include <q3valuelist.h>
#include <qkeysequence.h>
//Added by qt3to4:
#include <QKeyEvent>
#include "configobject.h"

/**
 * This class provides handling of keyboard events
 */

class ControlObject;

class MixxxKeyboard : public QObject
{
    Q_OBJECT
public:
    MixxxKeyboard(ConfigObject<ConfigValueKbd> *pKbdConfigObject, QObject *parent=0, const char *name=0);
    ~MixxxKeyboard();
    /** Event filter */
    bool eventFilter(QObject *obj, QEvent *e);

private:
    bool kbdPress(QKeySequence k, bool release, bool autoRepeat);
    /** Returns a valid QKeySequency with modifier keys from a QKeyEvent */
    QKeySequence getKeySeq(QKeyEvent *e);
    /** List containing keys which is currently pressed */
    Q3ValueList<int> m_qActiveKeyList;
    /** Pointer to keyboard config object */
    ConfigObject<ConfigValueKbd> *m_pKbdConfigObject;
};

#endif
