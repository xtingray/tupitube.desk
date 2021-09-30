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

#include "tupnewitemdialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>

TupNewItemDialog::TupNewItemDialog(QString &item, DialogType type, QSize size) : QDialog()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupNewItemDialog()]";
    #endif

    QFile file(THEME_DIR + "config/ui.qss");
    if (file.exists()) {
        file.open(QFile::ReadOnly);
        QString uiStyleSheet = QLatin1String(file.readAll());
        if (uiStyleSheet.length() > 0)
            setStyleSheet(uiStyleSheet);
        file.close();
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupNewItemDialog()] - theme file doesn't exist -> " << QString(THEME_DIR + "config/ui.qss");
        #endif
    }

    name = item;

    colors[0] = Qt::transparent;
    colors[1] = Qt::white;
    colors[2] = Qt::black;

    extension = new QComboBox();
    editor = new QComboBox();

    if (type == Raster) {
        setWindowTitle(tr("Create new raster item"));
        setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/bitmap.png")));
        extension->addItem("PNG");
        extension->addItem("JPG");
        fileExtension = "PNG";

        background = new QComboBox();
        background->addItem(tr("Transparent"));
        background->addItem(tr("White"));
        background->addItem(tr("Black"));
        bg = Qt::transparent;

#ifdef Q_OS_UNIX
        if (QFile::exists("/usr/bin/gimp"))
            editor->addItem("Gimp");
        if (QFile::exists("/usr/bin/krita"))
            editor->addItem("Krita");
        if (QFile::exists("/usr/bin/mypaint"))
            editor->addItem("MyPaint");
#endif
    } else {
        setWindowTitle(tr("Create new vector item"));
        setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/svg.png")));
        extension->addItem("SVG");
        editor->addItem("Inkscape");
        fileExtension = "SVG";
        software = "Inkscape";
    }

    software = editor->currentText();

    connect(extension, SIGNAL(currentIndexChanged(int)), this, SLOT(updateExtension(int)));
    connect(editor, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(updateEditor(const QString &)));

    QFormLayout *formLayout = new QFormLayout;

    itemName = new QLineEdit;
    itemName->setText(item);

    width = new QSpinBox;
    width->setMaximum(size.width());
    width->setMinimumWidth(60);
    width->setValue(50);

    height = new QSpinBox;
    height->setMaximum(size.height());
    height->setMinimumWidth(60);
    height->setValue(50);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok
                                | QDialogButtonBox::Cancel, Qt::Horizontal);
    connect(buttons, SIGNAL(accepted ()), this, SLOT(checkValues()));
    connect(buttons, SIGNAL(rejected ()), this, SLOT(reject()));

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(buttons);

    formLayout->addRow(tr("&Name:"), itemName);
    formLayout->addRow(tr("&Extension:"), extension);
    formLayout->addRow(tr("&Width:"), width);
    formLayout->addRow(tr("&Height:"), height);

    if (type == Raster) {
        formLayout->addRow(tr("&Background:"), background);
        connect(background, SIGNAL(currentIndexChanged(int)), this, SLOT(updateBackground(int)));
    }

    formLayout->addRow(tr("&Open it with:"), editor);
    formLayout->addRow(buttonsLayout);

    setLayout(formLayout);
}

TupNewItemDialog::~TupNewItemDialog()
{
}

void TupNewItemDialog::checkValues()
{
    QString text = itemName->text();
    if (text.length() == 0) {
        text = TAlgorithm::randomString(8);
        itemName->setText(text);
        return;
    }

    bool alert = false;

    if (width->value() == 0) {
        width->setValue(100);
        alert = true;
    }

    if (height->value() == 0) {
        height->setValue(100);
        alert = true;
    }

    if (alert)
        return;

    text.replace(" ", "_");
    text.replace(".", "_");
    name = text;
    size.setWidth(width->value());
    size.setHeight(height->value());

    accept();
}

void TupNewItemDialog::updateExtension(int index)
{
    fileExtension = extension->itemText(index);

    if (index == 1 || (index == 0 && software.compare("MyPaint") == 0)) {
        if (background->itemText(0).compare(tr("Transparent")) == 0)
            background->removeItem(0);
    } else {
        if (background->count() == 2)
            background->insertItem(0, tr("Transparent"));
    }
}

void TupNewItemDialog::updateBackground(int index)
{
    if (software.compare("MyPaint") == 0)
        bg = colors[index+1];
    else    
        bg = colors[index];
}

void TupNewItemDialog::updateEditor(const QString &editor)
{
    if (fileExtension.compare("SVG") == 0) {
        software = "Inkscape";
    } else {
        software = editor;
        if (editor.compare("MyPaint") == 0) {
            if (background->itemText(0).compare(tr("Transparent")) == 0)
                background->removeItem(0);
        } else {
            if (background->count() == 2)
                background->insertItem(0, tr("Transparent"));
        }
    }

}

QString TupNewItemDialog::getItemName() const
{
    return name;
}

QSize TupNewItemDialog::itemSize() const
{
    return size;
}

QString TupNewItemDialog::itemExtension() const
{
    return fileExtension;
}

QColor TupNewItemDialog::getBackground() const
{
    return bg;
}

QString TupNewItemDialog::getSoftware() const
{
    return software;
}
