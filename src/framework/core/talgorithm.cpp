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

#include "talgorithm.h"
#include "tconfig.h"
#include "tcachehandler.h"

#ifdef Q_OS_LINUX
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#endif

#include <QDate>
#include <QSettings>
#include <QTime>
#include <QSysInfo>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QCollator>
#include <QScreen>

int TAlgorithm::random()
{
    unsigned int seed;

    #ifdef Q_OS_LINUX
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd < 0 || ::read(fd, &seed, sizeof(seed)) != sizeof(seed)) {
            srand(getpid());
            seed = rand() + time(0);
        }
        if (fd >= 0)
            close(fd);
    #endif

    #if defined(Q_OS_MAC) || defined(Q_OS_WIN)
        QString day = QDate::currentDate().toString("d");
        QString time = QTime::currentTime().toString("mmzzz");
        QString number = day + time;
        if (day.toInt() % 2 == 0)
            number = time + day;

        int aux = number.toInt();
        seed = TCONFIG->value("RandomSeed", 0).toInt();
        if (seed == 0)
            seed = aux;
        seed *= aux;
    #endif

    TCONFIG->beginGroup("General");
    TCONFIG->setValue("RandomSeed", seed);

    // qsrand(seed);
    // return qrand();

    QRandomGenerator generator = QRandomGenerator(seed);
    return generator.generate();
}

QString TAlgorithm::randomString(int length)
{
    QString str;

    if (length <= 0) 
        return QString();

    QString date = QDate::currentDate().toString("ddddMMMMyyyy");
    QString input = date + QSysInfo::prettyProductName() + QTime::currentTime().toString("ssHHmmtzzz");	
    QByteArray hash = QCryptographicHash::hash(input.toUtf8(), QCryptographicHash::Md5);
    str = hash.toHex();
    str = str.left(length);
	
    return str;
}

QColor TAlgorithm::randomColor(bool withAlpha)
{
    QColor c(random() % 255, random() % 255, random() % 255);

    if (withAlpha)
        c.setAlpha(random() % 255);

    return c;
}

int TAlgorithm::randomNumber(int limit)
{
    return abs(random() % limit);
}

void TAlgorithm::storeData(const QString &data)
{
    TCONFIG->beginGroup("General");
    int id = TCONFIG->value("ClientID").toInt();
    TCacheHandler handler(id);
    QSettings settings(COMPANY, CACHE_DB);
    settings.setValue("cache", handler.getRecord(data));
}

bool TAlgorithm::cacheIDChanged(const QString &data)
{
    TCONFIG->beginGroup("General");
    int id = TCONFIG->value("ClientID").toInt();
    TCacheHandler handler(id);
    QString output = handler.getRecord(data);

    QSettings settings(COMPANY, CACHE_DB);
    QString reference = settings.value("cache").toString();
    if (reference.compare(output) == 0)
        return false;

    return true;
}

void TAlgorithm::resetCacheID()
{
    TCONFIG->beginGroup("Network");
    TCONFIG->setValue("Password", "");
    TCONFIG->setValue("StorePassword", false);
    TCONFIG->sync();

    QSettings settings(COMPANY, CACHE_DB);
    settings.setValue("cache", "");
}

QString TAlgorithm::windowCacheID()
{
    QSettings settings(COMPANY, CACHE_DB);
    QString reference = settings.value("cache").toString();
    TCONFIG->beginGroup("General");
    int id = TCONFIG->value("ClientID").toInt();
    TCacheHandler handler(id);

    return handler.setRecord(reference);
}

QStringList TAlgorithm::header(const QString &input)
{
    QStringList salts;
    salts << "0x9c1decb8$.ef28d34789ea2.f910b7cd7e6";
    salts << "0xda.695dcdc873555$929eb4bd.5c7da923d";
    salts << "0xc67a98dce7f0036$.7b8b0ce36a8.3d206c";
    QString prefix("$S$.");
    QStringList tokenList;

    for (int i = 0; i < salts.size(); ++i) {
         QString step1 = input + salts.at(i);
         QByteArray hash1 = QCryptographicHash::hash(step1.toUtf8(), QCryptographicHash::Sha1);
         QString step2 = hash1.toHex();
         QByteArray hash2 = QCryptographicHash::hash(step2.toUtf8(), QCryptographicHash::Md5);
         QString step3 = hash2.toHex();
         QByteArray hash3 = QCryptographicHash::hash(step3.toUtf8(), QCryptographicHash::Sha1);
         QString step4 = prefix + hash3.toHex();
         step4.insert(step4.length()/2, TAlgorithm::randomString(50).toLower());
         step4.insert(30, "-");
         tokenList << step4;
    }

    return tokenList;
}

bool TAlgorithm::isKeyRandomic(const QString &id)
{
    QChar pattern = id.at(0);
    int counter = 1;
    int size = id.size();
    for (int i = 1; i < size; ++i) {
        if (id.at(i) == pattern)
            counter++;
    }

    if (size == counter)
        return false;

    return true;
}

bool TAlgorithm::copyFolder(const QString &src, const QString &dst)
{
    QDir srcDir(src);
    if (!srcDir.exists()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TAlgorithm::copyFolder()] - "
                        "Fatal Error: Source folder doesn't exist -> " << src;
        #endif
        return false;
    }

    QDir dstDir(dst);
    if (!dstDir.exists()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TAlgorithm::copyFolder()] - "
                        "Fatal Error: Destination folder doesn't exist -> " << dst;
        #endif
        return false;
    }

    foreach (QString directory, srcDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString dstPath = dst + QDir::separator() + directory;
        srcDir.mkpath(dstPath);
        copyFolder(src + QDir::separator() + directory, dstPath);
    }

    foreach (QString file, srcDir.entryList(QDir::Files)) {
        QString sourceFile = src + QDir::separator() + file;
        QString targetFile = dst + QDir::separator() + file;
        #ifdef TUP_DEBUG
            qDebug() << "[TAlgorithm::copyFolder()] - sourceFile -> " << sourceFile;
            qDebug() << "[TAlgorithm::copyFolder()] - targetFile -> " << targetFile;
        #endif
        QFile::copy(sourceFile, targetFile);
    }

    return true;
}

QStringList TAlgorithm::naturalSort(QStringList elements)
{
    QCollator coll;
    coll.setNumericMode(true);
    for (int i = elements.size()-1; i >= 0; i--) {
         for (int j = 1; j <= i; j++) {
              if (coll.compare(elements.at(j-1), elements.at(j)) > 0)
                  elements.swapItemsAt(j-1, j);
         }
    }

    return elements;
}

float TAlgorithm::distance(const QPointF &p1, const QPointF &p2)
{
    float vx, vy;

    vx = p2.x() - p1.x();
    vy = p2.y() - p1.y();

    return sqrt((vx * vx) + (vy * vy));
}

float TAlgorithm::slope(const QPointF &p1, const QPointF &p2)
{
    return (p2.y() - p1.y())/(p2.x() - p1.x());
}

float TAlgorithm::inverseSlope(const QPointF &p1, const QPointF &p2)
{
    float m = (p2.y() - p1.y())/(p2.x() - p1.x());

    return -(1/m);
}

float TAlgorithm::calculateBFromLine(const QPointF &point, float slope)
{
    return (point.y() - (slope*point.x()));
}

float TAlgorithm::calculateYFromLine(float x, float m, float b)
{
    return (m*x + b);
}

float TAlgorithm::distanceFromLine(QPointF linePoint1, QPointF linePoint2, QPointF point)
{
    qreal top = fabs(((linePoint2.x() - linePoint1.x())*(linePoint1.y() - point.y()))
                     - ((linePoint1.x() - point.x())*(linePoint2.y() - linePoint1.y())));
    qreal bottom = sqrt(pow(linePoint2.x() - linePoint1.x(), 2) + pow(linePoint2.y() - linePoint1.y(), 2));

    return (top / bottom);
}

QPair<int, int> TAlgorithm::screenDimension()
{
    QPair<int, int> dimension;
    QScreen *screen = QGuiApplication::screens().at(0);
    QRect rect = screen->availableGeometry();
    dimension.first = rect.width();
    dimension.second = rect.height();

    return dimension;
}
