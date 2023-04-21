/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez                                                     *
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

#ifdef TUP_DEBUG

#include "tupcrashhandler.h"
#include "tupcrashwidget.h"
#include "tapptheme.h"

#include <QScreen>

#include <QScreen>
#include <QRegularExpression>

TupCrashHandler *TupCrashHandler::m_instance = 0;

void crashTrapper(int sig);

TupCrashHandler::TupCrashHandler()
{
    m_program = QCoreApplication::applicationName();
    setTrapper(crashTrapper);

    m_config.title = QObject::tr("Fatal Error");
    m_config.message = QObject::tr("Well, TupiTube has crashed...");
    m_config.closeButton = QObject::tr("Close");
    m_config.launchButton = QObject::tr("Re-launch TupiTube");
    m_config.defaultText = QObject::tr("This is a general failure");
    m_config.defaultImage = "crash.png";

    QPair<QString, QString> record;

    record.first = "<p align=\"justify\"><b>Signal 6:</b> The process itself has found that some essential pre-requisite for correct "
                   "function is not available and voluntarily killing itself.</p>";
    record.second = "crash_6.png";
    m_config.signalEntry[6] = record; 

    record.first = "<p align=\"justify\"><b>Signal 11:</b> Officially known as \"<i>segmentation fault</i>\", means that the program "
                   "accessed a memory location that was not assigned. That's usually a bug in the program.</p>";
    record.second = "crash_11.png";
    m_config.signalEntry[11] = record; 
}

TupCrashHandler::~TupCrashHandler()
{
    if (m_instance) 
        delete m_instance;
}

TupCrashHandler *TupCrashHandler::instance()
{
    init();
    return m_instance;
}
            
void TupCrashHandler::init()
{
    if (m_instance == 0)
        m_instance = new TupCrashHandler();
}

void TupCrashHandler::setTrapper (void (*trapper)(int))
{
    if (!trapper)
        trapper = SIG_DFL;

    sigset_t mask;
    sigemptyset(&mask);
    signal(SIGSEGV,trapper);
    sigaddset(&mask, SIGSEGV);
    signal(SIGFPE,trapper);
    signal(SIGILL,trapper);
    sigaddset(&mask, SIGILL);
    signal(SIGABRT, trapper);
    sigaddset(&mask, SIGABRT);
    signal(SIGBUS,trapper);
    signal(SIGIOT,trapper);
    sigprocmask(SIG_UNBLOCK, &mask, 0);
}

QString TupCrashHandler::program() const
{
    return m_program;
}

void TupCrashHandler::setProgram(const QString &prog)
{
    m_program = prog;
}

void TupCrashHandler::setImagePath(const QString &imagePath)
{
    m_imagePath = imagePath;
}

QString TupCrashHandler::imagePath() const
{
    return m_imagePath;
}

QString TupCrashHandler::title() const
{
    return m_config.title;
}

QString TupCrashHandler::message() const
{
    return m_config.message;
}

QColor TupCrashHandler::messageColor() const
{
    if (m_config.messageColor.isValid())
        return m_config.messageColor;

    return QApplication::palette().color(QPalette::Text);
}

QString TupCrashHandler::closeButtonLabel() const
{
    return m_config.closeButton;
}

QString TupCrashHandler::launchButtonLabel() const
{
    return m_config.launchButton;
}

QString TupCrashHandler::defaultText() const
{
    return m_config.defaultText;
}

QString TupCrashHandler::defaultImage() const
{
    QString image(m_imagePath + "/" + m_config.defaultImage);
    return image;
}

QString TupCrashHandler::signalText(int signal)
{
    QString data(m_config.signalEntry[signal].first);
    return data;
}

QString TupCrashHandler::signalImage(int signal)
{
    QString image(m_imagePath + m_config.signalEntry[signal].second);
    return image;
}

bool TupCrashHandler::containsSignalEntry(int signal)
{
    return m_config.signalEntry.contains(signal);
}

void TupCrashHandler::setConfig(const QString &filePath)
{
    // T_FUNCINFO;

    QDomDocument doc;
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly))
        return;

    if (!doc.setContent(&file)) {
        file.close();
        return;
    }
    file.close();

    QDomElement docElem = doc.documentElement();

    if (docElem.tagName() == "CrashHandler") {
        QDomNode n = docElem.firstChild();
        while (!n.isNull()) {
           QDomElement e = n.toElement();
           if (!e.isNull()) {
               if (e.tagName() == "Title") {
                   m_config.title = e.attribute("text");
               } else if (e.tagName() == "Message") {
                   m_config.message = e.attribute("text");
                   m_config.messageColor = QColor(e.attribute("color"));
               } else if (e.tagName() == "CloseButton") {
                   m_config.closeButton = e.attribute("text");
               } else if (e.tagName() == "Default") {
                   m_config.defaultText = "<p align=\"justify\">" + e.attribute("text") + "</p>";
                   m_config.defaultImage = e.attribute("image");
               } else if (e.tagName() == "Signal") {
                   int signalId = e.attribute("id").toInt();
                   m_config.signalEntry.insert(signalId, qMakePair(e.attribute("text"), e.attribute("image")));
               }
           }
           n = n.nextSibling();
        }
    }
}

QString loadStyle()
{
    return TAppTheme::themeSettings();
}

static QString runCommand(const QString &command, QStringList parameters)
{
    qDebug() << "Running command: " << command << parameters;

    QProcess process;
    process.start(command, parameters);
    if (!process.waitForStarted()) {
        qDebug() << "TupCrashHandler::runCommand() - Fatal Error: Can't run command -> " << command;
        return "FAILED TO START";
    }

    if (!process.waitForFinished())
        return "FAILED TO END";
    QByteArray result = process.readAll();

    qDebug() << "";
    qDebug() << "OUTPUT:";
    qDebug() << result;

    return result;
}

void crashTrapper(int sig)
{
    qDebug("\n*** Fatal Error: %s is crashing with signal %d :(", CHANDLER->program().toLocal8Bit().data(), sig);

    if (sig == 6) {
        qDebug("Signal 6: The process itself has found that some essential pre-requisite");
        qDebug("for correct function is not available and voluntarily killing itself.");
    }

    if (sig == 11) {
        qDebug("Signal 11: Officially known as \"segmentation fault\", means that the program");
        qDebug("accessed a memory location that was not assigned. That's usually a bug in the program.");
    }

    qDebug() << "";

    CHANDLER->setTrapper(0); // Unactive crash handler
    const pid_t pid = ::fork();

    QString SUDO = "/usr/bin/sudo";
    QString GDB = "/usr/bin/gdb";

    QString bt = "We are sorry. No debugging symbols were found :(";
    QString execInfo;

    // so we can read stderr too
    ::dup2(fileno(stdout), fileno(stderr));

    int appPID = ::getppid();
    QString pidStr = QString::number(appPID);

    if (QFile::exists(SUDO) && QFile::exists(GDB)) {
        QStringList parameters;
        parameters << GDB << "-n" << "-nw" << "-batch" << "-ex" << "where" << BIN_DIR << "tupitube.bin" << QString("--pid=" + pidStr);

        bt = runCommand(SUDO, parameters);

        // clean up
        static QRegularExpression re("\\(no debugging symbols found\\)");
        bt.remove(re);
        bt = bt.simplified();

        QStringList argument;
        argument << QString(BIN_DIR + "tupitube.bin");

        execInfo = runCommand("/usr/bin/file", argument);

        /*
           SQA: Find the crash error in this piece of code
           qDebug() << "*** Launching debugging dialog...";
           QScreen *screen = QGuiApplication::screens().at(0);
           TupCrashWidget widget(loadStyle(), sig);
           widget.setPid(::getpid());
           widget.addBacktracePage(execInfo, bt);
           widget.exec();
           widget.move(static_cast<int> ((screen->geometry().width() - widget.width()) / 2),
                       static_cast<int> ((screen->geometry().height() - widget.height()) / 2));
        */

        // Process crashed!
        ::alarm(0);
        // wait for child to exit
        ::waitpid(pid, NULL, 0);
    }

    ::exit(255);
}

#endif
