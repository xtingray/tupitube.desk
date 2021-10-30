/***************************************************************************
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

#ifndef TUPCUSTOMIZEDMOUTHVIEW_H
#define TUPCUSTOMIZEDMOUTHVIEW_H

#include <QWidget>

#include "tglobal.h"
#include "tuplipsyncdoc.h"

class TUPITUBE_EXPORT TupCustomizedMouthView : public QWidget
{
    Q_OBJECT

    public:
        explicit TupCustomizedMouthView(QWidget *parent = nullptr);
        ~TupCustomizedMouthView();

        void setDocument(TupLipsyncDoc *doc);
        void loadImages(const QString &folderPath);

    public slots:
        void onFrameChanged(int frame);

    protected:
        void paintEvent(QPaintEvent *event);

    private:
        TupLipsyncDoc *document;
        int32 frame;
        QHash<QString, QImage *> mouths;
        bool assetsLoaded;
};

#endif // TUPCUSTOMIZEDMOUTHVIEW_H
