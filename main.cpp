/*
 * Author: Brad Rott
 * Version: 0.9
 *
*/

#include "../headers/mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication::setOrganizationName("BNT Candles");
    QApplication::setApplicationName("BNT Candles");
    QApplication a(argc, argv);

    // Load stylesheet
    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream styleStream(&styleFile);
        QString styleSheet = styleStream.readAll();
        a.setStyleSheet(styleSheet);
        styleFile.close();
    } else {
        qDebug() << "Failed to load stylesheet";
    }

    BNTcandles window;
    window.show();
    return a.exec();
}
