#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator, base;
    if (translator.load(":tr/wrTime_ru.qm"))  a.installTranslator(&translator);
    if (base.load(":tr/qtbase_ru.qm")) a.installTranslator(&base);

    MainWindow w;
    w.show();

    return a.exec();
}
