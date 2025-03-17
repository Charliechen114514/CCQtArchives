#include "MainWindow.h"
#include <QPushButton>
#include <QProgressBar>
#include "CCMovableWidget.h"
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setMouseTracking(true);
    CCMovableWidget *moveWidget1 = new CCMovableWidget(this);
    QPushButton     *btn1        = new QPushButton(this);
    btn1->setGeometry(10, 10, 250, 25);
    btn1->setText("按住我拖动(仅限左键拖动)");
    moveWidget1->setMovingWidget(btn1);

    QPushButton *btn2 = new QPushButton(this);
    btn2->setGeometry(10, 40, 250, 25);
    btn2->setText("按住我拖动(支持右键拖动)");
    CCMovableWidget *moveWidget2 = new CCMovableWidget(this);
    moveWidget2->setMovingWidget(btn2);
    moveWidget2->installAcceptableMouseButtons(Qt::RightButton);

    QProgressBar *bar = new QProgressBar(this);
    bar->setGeometry(10, 70, 250, 25);
    bar->setRange(0, 0);
    bar->setTextVisible(false);
    CCMovableWidget *moveWidget3 = new CCMovableWidget(this);
    moveWidget3->setMovingWidget(bar);
}

MainWindow::~MainWindow() {
}
