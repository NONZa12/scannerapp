#include <QApplication>

#include "MainWindow/MainWindow.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);

    a.setStyle("fusion");

    MainWindow w;
    w.show();

    return a.exec();
}
