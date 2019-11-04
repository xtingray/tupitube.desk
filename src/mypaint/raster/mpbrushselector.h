/*
    Copyright © 2015 by The QTMyPaint Project

    This file is part of QTMyPaint, a Qt-based interface for MyPaint C++ library.

    QTMyPaint is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QTMyPaint is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QTMyPaint. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef MPBRUSHSELECTOR_H
#define MPBRUSHSELECTOR_H

#include <QTabWidget>
#include <QMap>
#include <QString>

class QListWidgetItem;

// MPBrushSelector is a TabWidget showing the various brushes (display the small screenshots)
// it allows the user to select it and emit a signal.
// NOTE : The order is not properly kept as I did not realize the file order.conf
//        was containing this information. Will be fixed soon.
//
class MPBrushSelector : public QTabWidget
{
    Q_OBJECT

    public:
      MPBrushSelector(const QString &brushLibPath, QWidget *parent = nullptr);

      bool isValid() { return !brushLib.isEmpty(); }

    public slots:
      void selectBrush(QString brushName = QString()); // Give the brush name (no extension) i.e. : "classic/blend+paint"

    signals:
      void brushSelected(const QByteArray &content);

    protected:
      QMap<QString, QStringList> brushLib;
      const QString brushesPath;

    protected slots:
      void itemClicked(QListWidgetItem *);
};

#endif // MPBRUSHSELECTOR_H
