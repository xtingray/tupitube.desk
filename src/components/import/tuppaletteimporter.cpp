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

#include "tuppaletteimporter.h"

TupPaletteImporter::TupPaletteImporter()
{
}

TupPaletteImporter::~TupPaletteImporter()
{
}

bool TupPaletteImporter::import(const QString &file, PaletteType type)
{
    switch (type) {
       case Gimp:
       {
           return importGimpPalette(file);
       }
       break;
    }

    return false;
}

bool TupPaletteImporter::importGimpPalette(const QString &file)
{
    QFile input(file);

    if (input.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&input);

        QFileInfo info(file);
        QString extension = info.suffix().toLower();

        if (extension.compare("gpl") == 0) {
            if (! stream.readLine().contains("GIMP Palette")) {
                #ifdef TUP_DEBUG
                    qDebug() << "TupPaletteImporter::importGimpPalette() - Fatal Error: Invalid palette format (.gpl) -> " + file;
                #endif
                return false;
            }

            QString line = stream.readLine();
            if (!line.startsWith("Name:")) {
                #ifdef TUP_DEBUG
                    qDebug() << "TupPaletteImporter::importGimpPalette() - Fatal Error: Invalid palette format (.gpl) -> " + file;
                #endif
                return false;
            }

            paletteName = line.section("Name:", 1).trimmed();
            document = new TupPaletteDocument(paletteName, false);

            line = stream.readLine();
            while(!line.contains("#"))
                  line = stream.readLine();

            while (!stream.atEnd()) {
                   line = stream.readLine().left(11).trimmed();
                   QStringList rgb = getColorArray(line);

                   if (rgb.count() != 3) {
                       #ifdef TUP_DEBUG
                           qDebug() << "TupPaletteImporter::importGimpPalette() - Fatal Error: Invalid palette format (.gpl) -> " + line;
                       #endif
                       return false;
                   }

                   QColor color;
                   for (int i = 0; i < 3; ++i) {
                        bool ok;
                        int item = rgb.at(i).toInt(&ok, 10);
                        if (ok) {
                            if (i == 0)
                                color.setRed(item);
                            if (i == 1)
                                color.setGreen(item);
                            if (i == 2)
                                color.setBlue(item);
                        } else {
                            #ifdef TUP_DEBUG
                                qDebug() << "TupPaletteImporter::importGimpPalette() - Fatal Error: Invalid palette format (.gpl) -> " + line;
                            #endif
                            return false;
                        }
                   }

                   if (color.isValid()) {
                       document->addColor(color);
                   } else {
                       #ifdef TUP_DEBUG
                           qDebug() << "TupPaletteImporter::importGimpPalette() - Fatal Error: Invalid color format (.gpl) -> " + line;
                       #endif
                       return false;
                   }
            }
        } else {
            paletteName = info.baseName().toLower();
            document = new TupPaletteDocument(paletteName, false);

            if (extension.compare("txt") == 0) {
                while (!stream.atEnd()) {
                       QString line = stream.readLine();
                       if (line.startsWith("#")) {
                           QColor color(line);
                           if (color.isValid()) {
                               document->addColor(color);
                           } else {
                               #ifdef TUP_DEBUG
                                   qDebug() << "TupPaletteImporter::importGimpPalette() - Fatal Error: Invalid color format (.txt) -> " + line;
                               #endif
                               return false;
                           }
                       }
                }
            } else if (extension.compare("css") == 0) {
                       while (!stream.atEnd()) {
                              QString line = stream.readLine();
                              int init = line.indexOf("(") + 1;
                              int end = line.indexOf(")");
                              if (init == -1 || end == -1) {
                                  #ifdef TUP_DEBUG
                                      qDebug() << "TupPaletteImporter::importGimpPalette() - Fatal Error: Invalid palette format (.css) -> " + line;
                                  #endif
                                  return false;
                              }

                              QString text = line.mid(init, end - init); 
                              QStringList rgb = text.split(",");
                              if (rgb.count() != 3) {
                                  #ifdef TUP_DEBUG
                                      qDebug() << "TupPaletteImporter::importGimpPalette() - Fatal Error: Invalid palette format (.css) -> " + line;
                                  #endif
                                  return false;
                              }

                              QColor color;
                              for (int i = 0; i < 3; ++i) {
                                   bool ok;
                                   int item = rgb.at(i).trimmed().toInt(&ok, 10);
                                   if (ok) {
                                       if (i == 0)
                                           color.setRed(item);
                                       if (i == 1)
                                           color.setGreen(item);
                                       if (i == 2)
                                           color.setBlue(item);
                                   } else {
                                       #ifdef TUP_DEBUG
                                           qDebug() << "TupPaletteImporter::importGimpPalette() - Fatal Error: Invalid palette format (.css) -> " + line;
                                       #endif
                                       return false;
                                   }
                              }

                              if (color.isValid()) {
                                  document->addColor(color);
                              } else {
                                  #ifdef TUP_DEBUG
                                      qDebug() << "TupPaletteImporter::importGimpPalette() - Fatal Error: Invalid color format (.css) -> " + line;
                                  #endif
                                  return false;
                              }
                       }
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "TupPaletteImporter::importGimpPalette() - Fatal Error: Invalid extension! -> " + extension;
                #endif
                return false;
            }
        }
    }

    return true;
}

bool TupPaletteImporter::saveFile(const QString &path)
{
    #ifdef TUP_DEBUG
        qWarning() << "TupMainWindow::saveFile() - Saving palette -> " + path;
    #endif

    if (paletteName.isNull()) {
        #ifdef TUP_DEBUG
            qDebug() << "TupPaletteImporter::saveFile() - Fatal Error: Palette name is null!";
        #endif
        return false;
    }

    QString pathName = path + "/" + paletteName.replace(" ", "_") + ".tpal";
    QFile file(pathName);
    if (file.exists()) {
        #ifdef TUP_DEBUG
            qWarning() << "TupPaletteImporter::saveFile() - Warning: Palette file already exists! -> " + pathName;
        #endif
    }

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream ts(&file);
        ts << document->toString();
        filePath = pathName;
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "TupPaletteImporter::saveFile() - Fatal Error: Insufficient permission to save palette file -> " + pathName;
        #endif
        return false;
    }

    return true;
}

QString TupPaletteImporter::getFilePath() const
{
    return filePath;
}

QStringList TupPaletteImporter::getColorArray(const QString &line)
{
    QByteArray array = line.toLocal8Bit();
    QStringList rgb;
    QString color = "";
    bool gotIt = false;
    for (int i=0; i<array.size(); i++) {
         QChar letter(array.at(i));
         if (letter.isDigit()) {
             color += letter;
             if (gotIt)
                 gotIt = false;
             if (i == array.size() - 1)
                 rgb << color;
         } else {
                 if (!gotIt) {
                     rgb << color;
                     color = "";
                     gotIt = true;
                 }
         }
    }

    return rgb;
}
