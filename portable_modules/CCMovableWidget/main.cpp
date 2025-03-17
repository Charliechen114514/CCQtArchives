#include "MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow   w;
    w.resize(QSize(640, 480));
    w.show();
    return a.exec();
}
