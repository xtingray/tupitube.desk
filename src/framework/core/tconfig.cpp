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

#include "tconfig.h"

#include <QFile>

TConfig* TConfig::m_instance = nullptr;

TConfig::TConfig() : QObject()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TConfig()]";
    #endif

    QString base = QDir::homePath() + "/";
    configDirectory.setPath(base + "." + QCoreApplication::applicationName());

    if (!configDirectory.exists()) {
        isFirstTime = true;
        #ifdef TUP_DEBUG
            qWarning() << "[TConfig::TConfig()] - Config file doesn't exist. Creating path -> " << configDirectory.path();
        #endif

        if (!configDirectory.mkdir(configDirectory.path())) {
            #ifdef TUP_DEBUG
                qDebug() << "[TConfig::TConfig()] - Fatal Error: Can't create path -> " << configDirectory.path();
            #endif
        }
    } else {
        isFirstTime = false;
    }

    path = configDirectory.path() + "/" + QCoreApplication::applicationName().toLower() + ".cfg";
    checkConfigFile();
}

TConfig::~TConfig()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TConfig()]";
    #endif

    if (m_instance) 
        delete m_instance;
}

TConfig *TConfig::instance()
{
    if (!m_instance)
        m_instance = new TConfig;

    return m_instance;
}

void TConfig::checkConfigFile()
{
    QFile config(path);
    isConfigOk = false;

    if (config.exists()) {
        QString errorMsg = "";
        int errorLine = 0;
        int errorColumn = 0;

        isConfigOk = domDocument.setContent(&config, &errorMsg, &errorLine, &errorColumn);
        if (!isConfigOk) {
            #ifdef TUP_DEBUG
                qDebug() << "[TConfig::checkConfigFile()] - Fatal Error: Configuration file is corrupted - Line -> "
                         << errorLine << " - Column -> " << errorColumn;
                qDebug() << "[TConfig::checkConfigFile()] - Message -> " << errorMsg;
            #endif
        } else {
            if (configVersion() < QString(CONFIG_VERSION).toInt())
                isConfigOk = false;
        }

        config.close();
    }

    if (!isConfigOk)
        initConfigFile();
}

void TConfig::initConfigFile()
{
    domDocument.clear();
    QDomProcessingInstruction header = domDocument.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    domDocument.appendChild(header);

    QDomElement root = domDocument.createElement("Config");
    root.setAttribute("version", CONFIG_VERSION);
    domDocument.appendChild(root);

    isFirstTime = true;
    isConfigOk = true;
}

int TConfig::configVersion()
{
   QDomElement root = domDocument.documentElement();
   int version = root.attribute("version", nullptr).toInt();

   return version;
}

bool TConfig::firstTime()
{
    return isFirstTime;
}

bool TConfig::isOk()
{
    return isConfigOk;
}

QDomDocument TConfig::document()
{
    return domDocument;
}

void TConfig::sync()
{
    QFile file(path);

    if (file.open(QIODevice::WriteOnly)) {
        QTextStream st(&file);
        st << domDocument.toString() << Qt::endl;
        isConfigOk = true;
        file.close();
    } else {
        isConfigOk = false;
    }

    checkConfigFile();
}

void TConfig::beginGroup(const QString &prefix)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TConfig::beginGroup()] - prefix -> " << prefix;
    #endif
    */

    QString stripped = QString(prefix).toHtmlEscaped();
    stripped.replace(' ', "_");
    stripped.replace('\n', "");

    lastGroup = currentElementsGroup.tagName();

    currentElementsGroup = find(domDocument.documentElement(), stripped);
    if (currentElementsGroup.isNull()) {
        currentElementsGroup = domDocument.createElement(stripped);
        domDocument.documentElement().appendChild(currentElementsGroup);
    }
}

void TConfig::endGroup()
{
    if (!lastGroup.isEmpty())
        beginGroup(lastGroup);
}

void TConfig::setValue(const QString &key, const QVariant &value)
{
    QDomElement element = find(currentElementsGroup, key);

    if (!element.isNull()) {
        if (value.canConvert(QVariant::StringList)) {
            QStringList list = value.toStringList();
            element.setAttribute("value", list.join(";"));
        } else {
            element.setAttribute("value", value.toString());
        }
    } else {
        element = domDocument.createElement(key);

        if (value.canConvert(QVariant::StringList)) {
            QStringList list = value.toStringList();
            element.setAttribute("value", list.join(";"));
        } else {
            element.setAttribute("value", value.toString());
        }

        currentElementsGroup.appendChild(element);
    }
}

QVariant TConfig::value(const QString &key, const QVariant &defaultValue) const
{
    QDomElement element = find(currentElementsGroup, key); // Current group or root?
    if (element.isNull())
        return defaultValue;

    QVariant content = element.attribute("value");
    if (content.toString() == "false") {
        return false;
    } else if (content.toString() == "true") {
        return true;
    }

    return content;
}

QDomElement TConfig::find(const QDomElement &element, const QString &key) const 
{
    QDomElement recent;
    QDomNode n = element.firstChild();

    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull()) {
            if (e.tagName() == key) {
                recent = e;
                break;
            }
        }
        n = n.nextSibling();
    }

    return recent;
}

QString TConfig::currentGroup()
{
    return lastGroup;
}

QStringList TConfig::languages()
{
    return {"zh_CN", "zh_TW", "en", "fr", "pt", "es"};
}

QStringList TConfig::timeRanges()
{
    return {"3", "5", "8", "10", "12"};
}
