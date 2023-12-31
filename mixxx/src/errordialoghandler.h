/***************************************************************************
                          errordialoghandler.h  -  description
                             -------------------
    begin                : Fri Feb 20 2009
    copyright            : (C) 2009 by Sean M. Pappalardo
    email                : pegasus@c64.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ERRORDIALOGHANDLER_H
#define ERRORDIALOGHANDLER_H

#include <QObject>
#include <QMessageBox>
#include <QSignalMapper>
#include <QMutex>

/**
  * Class used to allow all threads to display message boxes on error conditions
  *     with a custom list of standard buttons and to be able to react to them
  *
  *@author Sean M. Pappalardo
  */

typedef enum {
    DLG_FATAL       = 5,
    DLG_CRITICAL    = 4,
    DLG_WARNING     = 3,
    DLG_INFO        = 2,
    DLG_QUESTION    = 1,
    DLG_NONE        = 0 // No icon (default)
} DialogType;

class ErrorDialogProperties {
    public:
        // Befriending ErrorDialogHandler allows it to have cleaner code
        //  since the two are closely related anyway
        friend class ErrorDialogHandler;
        
        /** Set the window title. ("Mixxx" is always prepended.) */
        void setTitle(QString title);
        /** Set a key to prevent multiple dialogs until the first is closed */
        void setKey(QString key) { m_key = key; }
        QString getKey() { return m_key; }
        /** Set the primary window text */
        void setText(QString text);
        QString getText() { return m_text; }
        /** Set additional window text */
        void setInfoText(QString text) { m_infoText = text; }
        /** Set detailed text (causes "Show Details" button to appear.) */
        void setDetails(QString text) { m_details = text; }
        /** Set whether the box is modal (blocks the GUI) or not */
        void setModal(bool modal) { m_modal = modal; }
        /** Automatically sets the icon to match the type for convenience. */
        void setType(DialogType typeToSet);
        /** Set the box's icon to one of the standard ones */
        void setIcon(QMessageBox::Icon icon) { m_icon = icon; }
        
        /** Add a standard button to the box */
        void addButton(QMessageBox::StandardButton);
        /** Set the default button to highlight */
        void setDefaultButton(QMessageBox::StandardButton button) { m_defaultButton = button; }
        /** Set the button to click if the Escape key is pressed */
        void setEscapeButton(QMessageBox::StandardButton button) { m_escapeButton = button; }

    protected:
        // Protected since only ErrorDialogHandler should instantiate this
        ErrorDialogProperties();
        
        QString m_title;
        QString m_key;
        QString m_text;
        QString m_infoText;
        QString m_details;
        bool m_modal;
        DialogType m_type;
        QMessageBox::Icon m_icon;
        /** List of standard buttons to add to the box, in order
            Note that a QMessageBox::Ignore button, if clicked, will suppress
            error boxes with the same key for the duration of the application. */
        QList<QMessageBox::StandardButton> m_buttons;
        /** The default button to highlight, if any. */
        QMessageBox::StandardButton m_defaultButton;
        /** The button that's clicked if the Escape key is pressed. */
        QMessageBox::StandardButton m_escapeButton;
};

/** Singleton class because we only need one Handler to manage all error dialogs */
class ErrorDialogHandler : public QObject {
   Q_OBJECT
public:
    static ErrorDialogHandler* instance() {
        if (!s_pInstance)
            s_pInstance = new ErrorDialogHandler;
        return s_pInstance;
    }

    ~ErrorDialogHandler();
    /** Call this to get a new instance of ErrorDialogProperties to populate with data */
    ErrorDialogProperties* newDialogProperties();
    /** A qMessageHandler or any thread calls either of these to emit a signal to display the requested message box */
    /** They return false if a dialog with the same key (or title if no key) is already displayed */
    bool requestErrorDialog(DialogType type, QString message);
    bool requestErrorDialog(ErrorDialogProperties* props);
    /** Allows a means for main() to skip exec() if there was a critical or fatal error dialog displayed on app initialization */
    bool checkError();
    
signals:
    void showErrorDialog(ErrorDialogProperties* props);
    void stdButtonClicked(QString key, QMessageBox::StandardButton whichStdButton);

private:
    ErrorDialogHandler();                                       // Private constructor
    ErrorDialogHandler(const ErrorDialogHandler&);              // Prevent copy-construction
    ErrorDialogHandler& operator=(const ErrorDialogHandler&);   // Prevent assignment
    
    static ErrorDialogHandler *s_pInstance;
    
    bool m_errorCondition;
    QList<QString> m_dialogKeys;
    QSignalMapper* m_pSignalMapper;
    QMutex m_mutex;

private slots:
    /** Actually displays the box */
    void errorDialog(ErrorDialogProperties* props);
    void boxClosed(QString key);
};

#endif
