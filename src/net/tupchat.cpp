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

#include "tupchat.h"

TupChat::TupChat(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    QGridLayout *layout = new QGridLayout(this);
    
    setWindowTitle("chat");
    
    browser = new QTextBrowser;
    browser->setOpenExternalLinks(true);
    layout->addWidget(browser, 0, 0);
    
    QHBoxLayout *box = new QHBoxLayout;
    
    lineEdit = new QLineEdit;
    box->addWidget(lineEdit);
    
    send = new QPushButton(tr("Send"));
    box->addWidget(send);
    
    layout->addLayout(box, 1, 0);

    lines = new QList<QString>();
    lines->append("");
    cursorDown = 0;
    cursorUp = 0;
    
    connect(lineEdit, SIGNAL(returnPressed()), send, SLOT(animateClick()));
    connect(send, SIGNAL(clicked()), this, SLOT(sendMessage()));
}

TupChat::~TupChat()
{
}

void TupChat::addMessage(const QString &from, const QString &message)
{
    QTime time = QTime::currentTime();
    int hours = time.hour();
    QString h = QString::number(hours);
    if (hours < 10)
        h = "0" + h;

    int minutes = time.minute();
    QString min = QString::number(minutes);
    if (minutes < 10)
        min = "0" + min;

    QString record = "[" + h + ":" + min + "]";

    QString msg = formatMessage(message);

    QString css = "font-size: 12px;"; 
    browser->append("<div style=\"" + css + "\">" + record + " " + QString("<b>%1:</b>").arg(from) + " " + msg + "</div>");
}

QString TupChat::formatMessage(const QString &msg)
{
    QString base = msg;
    QString result = base;
    int index = base.indexOf("http://");

    if (index != -1) {
        bool flag = true;
        int count = 1;
        while (flag) {
               count++;
               int space = base.indexOf(" ", index);
               QString url;
               if (space != -1) {
                   url = base.mid(index, space);
                   result.insert(space, "</a>"); 
               } else {
                   space = base.length();
                   url = base.mid(index, space);
                   result.insert(space, "</a>");
               }

               QString last = url.right(1);
               while (!last[0].isLetter()) {
                      url.chop(1);
                      last = url.right(1);
               }

               result.insert(index, "<a href=\"" + url + "\">");
               space = result.lastIndexOf("</a>") + 4;
               index = result.indexOf("http://", space);
               if (index == -1)
                   flag = false; 
               else
                   base = result;
        }
    } 

    return result;
}

void TupChat::sendMessage()
{
    QString text = lineEdit->text();
    lineEdit->clear();

    if (!text.isEmpty()) {
        if (text.toLower().indexOf("<") != -1 && text.toLower().indexOf(">") != -1) {
            QString css = "font-size: 10px;";
            browser->append("<div style=\"" + css + "\">" + "<b>" + tr("Error:") + "</b> "
                               + tr("Invalid Message. It won't be sent. Please, don't use HTML tags") 
                               + "</div>");
        } else {
            lines->append(text);
            cursorUp = lines->size()-1;
            cursorDown = cursorUp;
            emit requestSendMessage(text);
        }
    }
}

void TupChat::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case (Qt::Key_Up):
            if ((cursorDown >= 0) && (cursorDown <= lines->size()-1)) {
                QString text = lines->at(cursorDown);
                if (!text.isNull()) {
                    lineEdit->setText(text);
                    if (cursorDown == 0) {
                        cursorUp = cursorDown + 1;
                        cursorDown = lines->size() - 1;
                    } else {
                        if (cursorDown == lines->size() - 1)
                            cursorUp = 0;
                        else
                            cursorUp = cursorDown + 1;
                        cursorDown--;
                    }
                }
            }
        break;
        case (Qt::Key_Down):
            if ((cursorUp >= 0) && (cursorUp <= lines->size()-1)) {
                QString text = lines->at(cursorUp);
                if (!text.isNull()) {
                    lineEdit->setText(text);
                    if (cursorUp == lines->size() - 1) {
                        cursorUp = 0;
                        cursorDown = lines->size() - 2;
                    } else {
                        if (cursorUp == 0)
                            cursorDown = lines->size() - 1;
                        else
                            cursorDown = cursorUp - 1;
                        cursorUp++;
                    }
                }
            }
        break;
    }
}
