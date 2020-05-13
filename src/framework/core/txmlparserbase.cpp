/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez / xtingray                                          *
 *                                                                         *
 *   KTooN's versions:                                                     * 
 *                                                                         *
 *   2006:                                                                 *
 *    David Cuadrado                                                       *
 *    Jorge Cuadrado                                                       *
 *   2003:                                                                 *
 *    Fernado Roldan                                                       *
 *    Simena Dinas                                                         *
 *                                                                         *
 *   Copyright (C) 2010 Gustav Gonzalez - http://www.maefloresta.com       *
 *   License:                                                              *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "txmlparserbase.h"

TXmlParserBase::TXmlParserBase() : QXmlDefaultHandler()
{
}

TXmlParserBase::~TXmlParserBase()
{
}

void TXmlParserBase::initialize()
{
}

bool TXmlParserBase::startDocument()
{
    isParsing = true;
    tag = QString();
    docRoot = QString();
    readText = false;
    ignore = false;
    
    initialize();

    return true;
}

bool TXmlParserBase::endDocument()
{
    isParsing = false;
    return true;
}

bool TXmlParserBase::startElement(const QString& , const QString& , const QString& qname, const QXmlAttributes& atts)
{
    if (ignore)
        return true;
    
    if (docRoot.isEmpty())
        docRoot = qname;
    
    bool r = startTag(qname, atts);   
    tag = qname;
    
    return r;
}

bool TXmlParserBase::endElement(const QString &, const QString &, const QString &qname)
{
    return endTag(qname);
}

bool TXmlParserBase::characters(const QString &ch)
{
    if (ignore)
        return true;
    
    if (readText) {
        text(ch.simplified());
        readText = false;
    }
    
    return true;
}

bool TXmlParserBase::error(const QXmlParseException &exception)
{
    #ifdef TUP_DEBUG	
        qWarning() << exception.lineNumber() << "x" << exception.columnNumber() << ": " << exception.message();
        qWarning() << __PRETTY_FUNCTION__ << " Document: " << document;
    #else
        Q_UNUSED(exception);
    #endif

    return true;
}

bool TXmlParserBase::fatalError(const QXmlParseException &exception)
{
    #ifdef TUP_DEBUG
        qWarning() << exception.lineNumber() << "x" << exception.columnNumber() << ": " << exception.message();
        qWarning() << __PRETTY_FUNCTION__ << " Document: " << document;
    #else
        Q_UNUSED(exception);
    #endif

    return true;
}

void TXmlParserBase::setReadText(bool read)
{
    readText = read;
}

void TXmlParserBase::setIgnore(bool flag)
{
    ignore = flag;
}

QString TXmlParserBase::currentTag() const
{
    return tag;
}

QString TXmlParserBase::root() const
{
    return docRoot;
}

bool TXmlParserBase::parse(const QString &doc)
{
    QXmlSimpleReader reader;
    
    reader.setContentHandler(this);
    reader.setErrorHandler(this);
    
    QXmlInputSource xmlsource;
    xmlsource.setData(doc);
    
    document = doc;
    
    return reader.parse(&xmlsource);
}

bool TXmlParserBase::parse(QFile *file)
{
    if (!file->isOpen()) {
        if (! file->open(QIODevice::ReadOnly | QIODevice::Text)) {
            #ifdef TUP_DEBUG
                qDebug() << "TXmlParserBase::parse() - Error: Cannot open file -> " + file->fileName();
            #endif
            return false;
        }
    }
    
    return parse(QString::fromLocal8Bit(file->readAll()));
}
