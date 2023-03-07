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

#include "tupnewproject.h"
// #include "tupnetprojectmanagerparams.h"

#include "tformfactory.h"
#include "tconfig.h"
#include "tapplication.h"
#include "tosd.h"
#include "tapptheme.h"

//SQA: Add a field to define the project description 

TupNewProject::TupNewProject(QWidget *parent) : TabDialog(parent)
{
    setWindowIcon(QPixmap(THEME_DIR + "icons/new.png"));
    setWindowTitle(tr("Create New Project"));
    setModal(true);

    setStyleSheet(TAppTheme::themeSettings());

    QFrame *infoContainer = new QFrame();
    QGridLayout *layout = new QGridLayout(infoContainer);

    QLabel *nameLabel = new QLabel(tr("Project Name"), infoContainer);
    layout->addWidget(nameLabel, 0, 0);

    projectName = new QLineEdit(infoContainer);
    projectName->setMaxLength(30);
    projectName->setText(tr("my_project"));
    layout->addWidget(projectName, 0, 1);

    QLabel *authorLabel = new QLabel(tr("Author"), infoContainer);
    layout->addWidget(authorLabel, 1, 0);

    authorName = new QLineEdit(infoContainer);
    authorName->setMaxLength(30);
    authorName->setText(tr("Your name"));
    layout->addWidget(authorName, 1, 1);

    QLabel *descLabel = new QLabel(tr("Description"), infoContainer);
    layout->addWidget(descLabel, 3, 0);

    description = new QLineEdit(infoContainer);
    description->setMaxLength(50);
    description->setText(tr("Just for fun!"));
    layout->addWidget(description, 3, 1);

    QBoxLayout *presetsLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    QLabel *presetsLabel = new QLabel(tr("Presets") + " ");

    TCONFIG->beginGroup("PaintArea");
    int presetIndex = TCONFIG->value("DefaultFormat", FORMAT_1080).toInt();

    presets = new QComboBox();
    presets->addItem(tr("Free Format"));
    presets->addItem(tr("520x380 - 24"));
    presets->addItem(tr("640x480 - 24"));
    presets->addItem(tr("480 (PAL DV/DVD) - 25"));
    presets->addItem(tr("576 (PAL DV/DVD) - 25"));
    presets->addItem(tr("720 (HD) - 24"));
    presets->addItem(tr("1080 (Mobile) - 24"));
    presets->addItem(tr("1080 (Full HD Vertical) - 24"));
    presets->addItem(tr("1080 (Full HD) - 24"));

    connect(presets, SIGNAL(currentIndexChanged(int)), this, SLOT(setPresets(int)));

    presetsLayout->addWidget(presetsLabel);
    presetsLayout->addWidget(presets);
    layout->addLayout(presetsLayout, 4, 0, 1, 2, Qt::AlignCenter);

    QGroupBox *renderAndFps= new QGroupBox(tr("Options"));
	
    QBoxLayout *subLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    renderAndFps->setLayout(subLayout);

    TCONFIG->beginGroup("PaintArea");
    QString colorName = TCONFIG->value("BackgroundDefaultColor", "#ffffff").toString();

    color = QColor(colorName);

    colorButton = new QPushButton();
    colorButton->setText(tr("Background"));
    colorButton->setToolTip(tr("Click here to change background color"));
    colorButton->setStyleSheet("QPushButton { background-color: " + color.name()
                                + "; color: " + labelColor() + "; }");
    connect(colorButton, SIGNAL(clicked()), this, SLOT(setBgColor()));

    QBoxLayout *fpsLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    QLabel *label = new QLabel(tr("FPS"));
    fps = new QSpinBox();
    fps->setValue(24);

    fpsLayout->addWidget(label);
    fpsLayout->addWidget(fps);
    subLayout->addWidget(colorButton);
    subLayout->addLayout(fpsLayout);

    size = new TXYSpinBox(tr("Dimension"), tr("X:"), tr("Y:"), infoContainer);
    size->setMinimum(50);
    size->setMaximum(15000);
    size->setX(520);
    size->setY(380);

    connect(size, SIGNAL(valuesHaveChanged()), this, SLOT(updateFormatCombo()));

    QWidget *panel = new QWidget;
    QVBoxLayout *sizeLayout = new QVBoxLayout(panel);
    sizeLayout->addWidget(size);

    layout->addWidget(panel, 5, 0);
    layout->addWidget(renderAndFps, 5, 1);

    QCheckBox *activeNetOptions = new QCheckBox(tr("TupiTube project"));
    connect(activeNetOptions, SIGNAL(toggled(bool)), this, SLOT(enableNetOptions(bool)));

    // SQA: Code temporarily disabled
    // layout->addWidget(activeNetOptions, 5, 0, 1, 2, Qt::AlignLeft);

    addTab(infoContainer, tr("Project Info"));

    QFrame *netContainer = new QFrame();
    netLayout = new QBoxLayout(QBoxLayout::TopToBottom, netContainer);
    netLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    setupNetOptions();

    // SQA: Code temporarily disabled
    // addTab(netContainer, tr("Network"));
    enableNetOptions(false);

    if (presetIndex >= 0)
        presets->setCurrentIndex(presetIndex);
}

TupNewProject::~TupNewProject()
{
    if (enableUseNetwork) {
        TConfig *config = kApp->config("Network");
        config->setValue("Server", server->text());
        config->setValue("Port", port->value());
        config->setValue("Login", username->text());
        if (storePassword->isChecked()) {
            config->setValue("Password", password->text());
            config->setValue("StorePassword", "1");
        } else {
            config->setValue("Password", "");
            config->setValue("StorePassword", "0");
        }
    }

    /*
    SQA: delete this variables causes the app to crash when it creates a project

    delete projectName;
    delete authorName;
    delete tags;
    delete description;
    delete colorButton;
    delete fps;
    delete presets;
    delete size;
    delete netOptions;
    delete netLayout;
    delete server;
    delete port;
    delete username;
    delete password;
    delete storePassword;
    */
}

void TupNewProject::setupNetOptions()
{
    server = new QLineEdit;
    port = new QSpinBox;
    port->setMinimum(1024);
    port->setMaximum(65000);

    username = new QLineEdit;
    password = new QLineEdit;

    TConfig *config = kApp->config("Network");

    server->setText(config->value("Server", "tupitu.be").toString());
    port->setValue(config->value("Port", 8080).toInt());

    username->setText(config->value("Login", "").toString());
    password->setText(config->value("Password", "").toString());

    password->setEchoMode(QLineEdit::Password);

    QPlainTextEdit *text = new QPlainTextEdit;
    text->setMaximumHeight(70);
    text->appendPlainText(QString("This feature allows you to work with another artists") 
                           + QString(" around the Internet on the same project in real time."));
    text->setEnabled(false);
    netLayout->addWidget(text);

    netOptions = new QGroupBox(tr("Settings"));
    QVBoxLayout *layout = new QVBoxLayout(netOptions);
    layout->addLayout(TFormFactory::makeGrid(QStringList() << tr("Username") << tr("Password") << tr("Server") << tr("Port"), 
                         QWidgetList() << username << password << server << port));

    netLayout->addWidget(netOptions);

    storePassword = new QCheckBox(tr("Store password"));
    storePassword->setChecked(TCONFIG->value("StorePassword").toInt());
    netLayout->addWidget(storePassword);
}

TupProjectManagerParams *TupNewProject::parameters()
{
    int w = size->x();
    if (w % 2)
        w++;
    int h = size->y();
    if (h % 2)
        h++;

    /*
    if (enableUseNetwork) {
        TupNetProjectManagerParams *params = new TupNetProjectManagerParams;
        params->setProjectName(projectName->text());
        params->setAuthor(authorName->text());
        params->setDescription(description->text());
        params->setBgColor(color);
        const QSize projectSize(w, h);
        params->setDimension(projectSize);
        params->setFPS(fps->value());

        // Network settings
        params->setServer(server->text());
        params->setPort(port->value());
        params->setLogin(username->text());
        params->setPassword(password->text());

        return params;
    }
    */

    TupProjectManagerParams *params = new TupProjectManagerParams;
    params->setProjectName(projectName->text());
    params->setAuthor(authorName->text());
    params->setDescription(description->text());
    params->setBgColor(color);
    const QSize projectSize(w, h);
    params->setDimension(projectSize);
    params->setFPS(fps->value());

    return params;
}

bool TupNewProject::useNetwork() const
{
    return enableUseNetwork;
}

void TupNewProject::ok()
{
    if (projectName->text().isEmpty()) {
        TOsd::self()->display(TOsd::Error, tr("Please, set a name for the project"));
        return;
    }

    if (enableUseNetwork) {
        if (username->text().isEmpty()) {
            TOsd::self()->display(TOsd::Error, tr("Please, fill in your username"));
            return;
        }

        if (password->text().isEmpty()) {
            TOsd::self()->display(TOsd::Error, tr("Please, fill in your password"));
            return;
        }

        if (server->text().isEmpty()) {
            TOsd::self()->display(TOsd::Error, tr("Please, fill in the server name or IP"));
            return;
        }
    }

    TCONFIG->beginGroup("PaintArea");
    TCONFIG->setValue("BackgroundDefaultColor", color.name());
    TCONFIG->setValue("DefaultFormat", presets->currentIndex());
    TCONFIG->sync();

    TabDialog::ok();
}

void TupNewProject::enableNetOptions(bool isEnabled)
{
    enableUseNetwork = isEnabled;
    enableTab(1, isEnabled);
}

void TupNewProject::focusProjectLabel() 
{
    projectName->setFocus();
    projectName->selectAll();
}

void TupNewProject::setBgColor()
{
    color = QColorDialog::getColor(color, this);

    // SQA: what is this?
    // QString labelColorStr = "black";

     if (color.isValid()) {
         colorButton->setText(color.name());
         colorButton->setStyleSheet("QPushButton { background-color: " + color.name()
                                     + "; color: " + labelColor() + "; }");
     } else {
         color = QColor(Qt::white);
         colorButton->setText(tr("White"));
         colorButton->setStyleSheet("QPushButton { background-color: #fff }; color: black;");
     }
}

void TupNewProject::setPresets(int index)
{
    size->blockSignals(true);

    switch(index) {
           case FREE:
           break;
           case FORMAT_520:
           {
               size->setX(520);
               size->setY(380);
               fps->setValue(24);
           }
           break;
           case FORMAT_640:
           {
               size->setX(640);
               size->setY(480);
               fps->setValue(24);
           }
           break;
           case FORMAT_480:
           {
               size->setX(720);
               size->setY(480);
               fps->setValue(25);
           }
           break;
           case FORMAT_576:
           {
               size->setX(720);
               size->setY(576);
               fps->setValue(25);
           }
           break;
           case FORMAT_720:
           {
               size->setX(1280);
               size->setY(720);
               fps->setValue(24);
           }
           break;
           case FORMAT_MOBILE:
           {
               size->setX(1080);
               size->setY(1080);
               fps->setValue(24);
           }
           break;
           case FORMAT_1080_VERTICAL:
           {
               size->setX(1080);
               size->setY(1920);
               fps->setValue(24);
           }
           break;
           case FORMAT_1080:
           {
               size->setX(1920);
               size->setY(1080);
               fps->setValue(24);
           }
           break;
    }

    size->blockSignals(false);
}

QString TupNewProject::login() const
{
    return username->text();
}

void TupNewProject::updateFormatCombo()
{
    presets->blockSignals(true);
    presets->setCurrentIndex(0);
    presets->blockSignals(false);
}

QString TupNewProject::labelColor() const
{
    QString text = "white";
    if (color.red() > 50 && color.green() > 50 && color.blue() > 50)
        text = "black";
    return text;
}
