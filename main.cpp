#include "mainwindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QStringList>

#include "downloadmanager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
