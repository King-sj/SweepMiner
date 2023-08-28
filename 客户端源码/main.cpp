#include "MainWindow.h"
#include <QApplication>

#include <QLoggingCategory>
#include <QMessageLogger>
#include <QFile>
#include <QTextStream>
#include"Base.h"
void myMessageOutput(QtMsgType type,
                     const QMessageLogContext &context,
                     const QString &msg)
{
    QString txt;
    switch (type) {
    case QtDebugMsg:
        txt = QString("Debug: %1").arg(msg);
        break;
    case QtInfoMsg:
        txt = QString("Info: %1").arg(msg);
        break;
    case QtWarningMsg:
        txt = QString("Warning: %1").arg(msg);
        break;
    case QtCriticalMsg:
        txt = QString("Critical: %1").arg(msg);
        break;
    case QtFatalMsg:
        txt = QString("Fatal: %1").arg(msg);
        break;
    }

    QString logFileName = "./log.txt";
    QFile logFile(logFileName);
    if (!logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream out(&logFile);
    out << txt << "\n";
}
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    #ifdef QT_DEBUG
        dout << "Debug！\n";
    #else
        // Redirect qDebug() to log.txt file
        qInstallMessageHandler(myMessageOutput);
        dout << "Release!\n";
    #endif

    qApp->setWindowIcon(QIcon(":/img/Resource/img/game.ico"));
    MainWindow w;
    w.show();    
    return a.exec();
}
