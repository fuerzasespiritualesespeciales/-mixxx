/***************************************************************************
                          upgrade.cpp  -  description
                             -------------------
    begin                : Fri Mar 13 2009
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

#include <QtCore>
#include "defs_version.h"

#include "configobject.h"
#include "upgrade.h"

Upgrade::Upgrade()
{
    m_bFirstRun = false;
    m_bUpgraded = false;
}

Upgrade::~Upgrade()
{

}

// We return the ConfigObject here because we have to make changes to the
// configuration and the location of the file may change between releases.
ConfigObject<ConfigValue>* Upgrade::versionUpgrade() {

/*  Pre-1.7.0:
*
*   Since we didn't store version numbers in the config file prior to 1.7.0,
*   we check to see if the user is upgrading if his config files are in the old location,
*   since we moved them in 1.7.0. This code takes care of moving them.
*/

    QString oldLocation = QDir::homePath().append("/%1");
#ifdef __WINDOWS__
    QFileInfo* pre170Config = new QFileInfo(oldLocation.arg("mixxx.cfg"));
#else
    QFileInfo* pre170Config = new QFileInfo(oldLocation.arg(".mixxx.cfg"));
#endif

    if (pre170Config->exists()) {

        // Move the files to their new location
        QString newLocation = QDir::homePath().append("/").append(SETTINGS_PATH);
        
        if (!QDir(newLocation).exists()) {
            qDebug() << "Creating new settings directory" << newLocation;
            QDir().mkpath(newLocation);
        }
        
        newLocation.append("%1");
        QString errorText = "Error moving your %1 file %2 to the new location %3: \n";

#ifdef __WINDOWS__
        QString oldFilePath = oldLocation.arg("mixxxtrack.xml");
#else
        QString oldFilePath = oldLocation.arg(".mixxxtrack.xml");
#endif

        QString newFilePath = newLocation.arg("mixxxtrack.xml");
        QFile* oldFile = new QFile(oldFilePath);
        if (oldFile->exists()) {
            if (oldFile->copy(newFilePath)) {
                oldFile->remove();
                m_bUpgraded = true;
            }
            else {
                if (oldFile->error()==14) qDebug() << errorText.arg("library", oldFilePath, newFilePath) << "The destination file already exists.";
                else qDebug() << errorText.arg("library", oldFilePath, newFilePath) << "Error #" << oldFile->error();
            }
        }
        delete oldFile;
        
#ifdef __WINDOWS__
        oldFilePath = oldLocation.arg("mixxxbpmschemes.xml");
#else
        oldFilePath = oldLocation.arg(".mixxxbpmscheme.xml");
#endif
        newFilePath = newLocation.arg("mixxxbpmscheme.xml");
        oldFile = new QFile(oldFilePath);
        if (oldFile->exists()) {
            if (oldFile->copy(newFilePath))
                oldFile->remove();
            else {
                if (oldFile->error()==14) qDebug() << errorText.arg("settings", oldFilePath, newFilePath) << "The destination file already exists.";
                else qDebug() << errorText.arg("settings", oldFilePath, newFilePath) << "Error #" << oldFile->error();
            }
        }
        delete oldFile;
#ifdef __WINDOWS__
        oldFilePath = oldLocation.arg("MixxxMIDIBindings.xml");
#else
        oldFilePath = oldLocation.arg(".MixxxMIDIBindings.xml");
#endif
        newFilePath = newLocation.arg("MixxxMIDIBindings.xml");
        oldFile = new QFile(oldFilePath);
        if (oldFile->exists()) {
            qWarning() << "The MIDI mapping file format has changed in this version of Mixxx. You will need to reconfigure your MIDI controller. See the Wiki for full details on the new format.";
            if (oldFile->copy(newFilePath))
                oldFile->remove();
            else {
                if (oldFile->error()==14) qDebug() << errorText.arg("MIDI mapping", oldFilePath, newFilePath) << "The destination file already exists.";
                else qDebug() << errorText.arg("MIDI mapping", oldFilePath, newFilePath) << "Error #" << oldFile->error();
            }
        }
        // Tidy up
        delete oldFile;

        QFile::remove(oldLocation.arg(".MixxxMIDIDevice.xml")); // Obsolete file, so just delete it

#ifdef __WINDOWS__
        oldFilePath = oldLocation.arg("mixxx.cfg");
#else
        oldFilePath = oldLocation.arg(".mixxx.cfg");
#endif
        newFilePath = newLocation.arg(SETTINGS_FILE);
        oldFile = new QFile(oldFilePath);
        if (oldFile->copy(newFilePath))
            oldFile->remove();
        else {
                if (oldFile->error()==14) qDebug() << errorText.arg("configuration", oldFilePath, newFilePath) << "The destination file already exists.";
                else qDebug() << errorText.arg("configuration", oldFilePath, newFilePath) << "Error #" << oldFile->error();
            }
        delete oldFile;
        
    }
    // Tidy up
    delete pre170Config;
    // End pre-1.7.0 code
    
    
/***************************************************************************
*                           Post-1.7.0 upgrade code
*
*   Add entries to the IF ladder below if anything needs to change from the
*   previous to the current version. This allows for incremental upgrades
*   incase a user upgrades from a few versions prior.
****************************************************************************/

    // Read the config file from home directory
    ConfigObject<ConfigValue> *config = new ConfigObject<ConfigValue>(QDir::homePath().append("/").append(SETTINGS_PATH).append(SETTINGS_FILE));

    QString configVersion = config->getValueString(ConfigKey("[Config]","Version"));

    if (configVersion.isEmpty()) {
        qDebug() << "No version number in configuration file. Setting to" << VERSION;
        config->set(ConfigKey("[Config]","Version"), ConfigValue(VERSION));

        //This must have been the first run... right? :)
        m_bFirstRun = true;
        return config;
    }
    
    // If it's already current, stop here
    if (configVersion == VERSION) {
        qDebug() << "Configuration file is at the current version" << VERSION;
        return config;
    }

    // Allows for incremental upgrades incase someone upgrades from a few versions prior
    // (I wish we could do a switch on a QString.)
    /*
    // Examples, since we didn't store the version number prior to v1.7.0
    if (configVersion.startsWith("1.6.0")) {
        qDebug() << "Upgrading from v1.6.0 to 1.6.1...";
        // Upgrade tasks go here
        configVersion = "1.6.1";
        config->set(ConfigKey("[Config]","Version"), ConfigValue("1.6.1"));
    }
    if (configVersion.startsWith("1.6.1")) {
        qDebug() << "Upgrading from v1.6.1 to 1.7.0...";
        // Upgrade tasks go here
        configVersion = "1.7.0";
        config->set(ConfigKey("[Config]","Version"), ConfigValue("1.7.0"));
    }
    */

    //We use the following blocks to detect if this is the first time
    //you've run the latest version of Mixxx. This lets us show
    //the promo tracks stats agreement stuff for all users that are
    //upgrading Mixxx.
    
    if (configVersion.startsWith("1.7")) {
        qDebug() << "Upgrading from v1.7.x to" << VERSION <<"...";
        // Upgrade tasks go here
        // Nothing to change, really
    }

    if (configVersion.startsWith("1.8.0~beta1") || 
        configVersion.startsWith("1.8.0~beta2")) {
        qDebug() << "Upgrading from v1.8.0~beta to" << VERSION <<"...";
        // Upgrade tasks go here
    }
    // For the next release
    /*
    if (configVersion.startsWith("1.8.0")) {
        qDebug() << "Upgrading from v1.8.0 to" << VERSION <<"...";
        // Upgrade tasks go here, if any
        configVersion = VERSION;
        config->set(ConfigKey("[Config]","Version"), ConfigValue(VERSION));
    }
    */

    configVersion = VERSION;
    m_bUpgraded = true;
    config->set(ConfigKey("[Config]","Version"), ConfigValue(VERSION));

    if (configVersion == VERSION) qDebug() << "Configuration file is now at the current version" << VERSION;
    else {
        /* Way too verbose, this confuses the hell out of Linux users when they see this:
        qWarning() << "Configuration file is at version" << configVersion
                   << "and I don't know how to upgrade it to the current" << VERSION
                   << "\n   (That means a function to do this needs to be added to upgrade.cpp.)"
                   << "\n-> Leaving the configuration file version as-is.";
        */
    }

    return config;
}
