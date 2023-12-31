//
// C++ Implementation: parser
//
// Description: superclass for external formats parsers
//
//
// Author: Ingo Kossyk <kossyki@cs.tu-berlin.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "parser.h"
//Added by qt3to4:
#include <Q3PtrList>
#include <QtDebug>

/**
   @author Ingo Kossyk (kossyki@cs.tu-berlin.de)
 **/


Parser::Parser()
{
    m_psLocations = new Q3PtrList<QString>;
}

Parser::~Parser()
{


}

void Parser::clearLocations()
{
    while(!m_psLocations->isEmpty())
        m_psLocations->removeFirst();
}

long Parser::countParsed()
{
    return (long)m_psLocations->count();
}

bool Parser::isFilepath(QString sFilepath){
    QFile * file = new QFile(sFilepath);

    if(file->exists())

        return true;
    else

        return false;

}

bool Parser::isBinary(QString filename){
    QFile * file = new QFile(filename);

    if(file->open(QIODevice::ReadOnly)){
        char c;
        unsigned char uc;
        
        if(!file->getChar(&c))
        {
          qDebug() << "Parser: Error reading stream on " << filename;
          return true; //should this raise an exception?
        }
        
        uc = uchar(c);
        
        if(!(33<=uc && uc<=127))  //Starting byte is no character
        {
            file->close();
            return true;
        }

    } else{
        qDebug() << "Parser: Could not open file: " << filename;
    }
    //qDebug(QString("Parser: textstream starting character is: %1").arg(i));
    file->close();
    return false;
}
