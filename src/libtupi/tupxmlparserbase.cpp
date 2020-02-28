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

#include "tupxmlparserbase.h"

TupXmlParserBase::TupXmlParserBase() : QXmlDefaultHandler()
{
}

TupXmlParserBase::~TupXmlParserBase()
{
}

void TupXmlParserBase::initialize()
{
}

bool TupXmlParserBase::startDocument()
{
    isParsing = true;
    gTag = QString();
    rootStr = QString();
    readText = false;
    ignore = false;

    initialize();

    return true;
}

bool TupXmlParserBase::endDocument()
{
    isParsing = false;
    return true;
}

bool TupXmlParserBase::startElement(const QString& , const QString& , const QString& qname, const QXmlAttributes& atts)
{
     if (ignore)
         return true;
	
     if (rootStr.isEmpty())
         rootStr = qname;

     bool element = startTag(qname, atts);
     gTag = qname;

     return element;
}

bool TupXmlParserBase::endElement(const QString&, const QString& , const QString& qname)
{
     return endTag(qname);
}

bool TupXmlParserBase::characters(const QString & ch)
{
     if (ignore)
         return true;

     if (readText) {
         text(ch.simplified());
         readText = false;
     }

     return true;
}

bool TupXmlParserBase::error(const QXmlParseException & exception)
{
#ifdef TUP_DEBUG	
     qDebug() << exception.lineNumber() + QString("x") + exception.columnNumber() + QString(": ") + exception.message();
     qWarning() << " Document: " << document;
#else
     Q_UNUSED(exception);
#endif

     return true;
}

bool TupXmlParserBase::fatalError(const QXmlParseException & exception)
{
#ifdef TUP_DEBUG	
     qDebug() << exception.lineNumber() + QString("x") + exception.columnNumber() + QString(": ") + exception.message();
     qWarning() << " Document: " << document;
#else
     Q_UNUSED(exception);
#endif

     return true;
}

void TupXmlParserBase::setReadText(bool read)
{
     readText = read;
}

void TupXmlParserBase::setIgnore(bool flag)
{
     ignore = flag;
}

QString TupXmlParserBase::currentTag() const
{
     return gTag;
}

QString TupXmlParserBase::root() const
{
     return rootStr;
}

bool TupXmlParserBase::parse(const QString &doc)
{
     QXmlSimpleReader reader;

     reader.setContentHandler(this);
     reader.setErrorHandler(this);

     QXmlInputSource xmlsource;
     xmlsource.setData(doc);

     document = doc;

     return reader.parse(&xmlsource);
}

bool TupXmlParserBase::parse(QFile *file)
{
     if (!file->isOpen()) {
         if (! file->open(QIODevice::ReadOnly | QIODevice::Text)) {
             #ifdef TUP_DEBUG
                 qDebug() << "TupXmlParserBase::parse() - Cannot open file " + file->fileName();
             #endif

             return false;
         }
     }

     return parse(QString::fromLocal8Bit(file->readAll()));
}
