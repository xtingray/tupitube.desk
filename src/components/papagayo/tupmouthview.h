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

#ifndef TUPMOUTHVIEW_H
#define TUPMOUTHVIEW_H

#include <QWidget>

#include "tglobal.h"
#include "tuplipsyncdoc.h"

class TUPITUBE_EXPORT TupMouthView : public QWidget
{
	Q_OBJECT

    public:
        explicit TupMouthView(QWidget *parent = nullptr);
        ~TupMouthView();

        void setDocument(TupLipsyncDoc *doc);
        void setMouth(int32 id);

    public slots:
        void onMouthChanged(int id);
        void onFrameChanged(int frame);

    protected:
        void paintEvent(QPaintEvent *event);

    private:
        TupLipsyncDoc *document;
        int32 mouthID;
        int32 frame;
        QHash<QString, QImage *> mouths[5];
};

#endif // TUPMOUTHVIEW_H
