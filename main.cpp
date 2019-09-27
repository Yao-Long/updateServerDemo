#include "mainwindow.h"
#include <QApplication>
//#include <QDebug>
//#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    //qDebug()<<QTextCodec::availableCodecs();

    MainWindow w;
    w.show();

    return a.exec();
}
