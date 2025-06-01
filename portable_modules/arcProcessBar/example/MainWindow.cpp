#include "MainWindow.h"
#include "CCArcProgressWidget.h"
#include "ui_MainWindow.h"
#include <QRandomGenerator>
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
	ui->setupUi(this);
	arc = new CCArcProgressWidget(this);
	ui->centralwidget->layout()->addWidget(arc);
}

MainWindow::~MainWindow() {
	delete ui;
}

void MainWindow::on_btn_set_random_clicked() {
	arc->setValue(QRandomGenerator::global()->bounded(101));
}
