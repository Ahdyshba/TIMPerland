#include "homewindow.h"
#include "ui_homewindow.h"


HomeWindow::~HomeWindow()
{
    delete ui;
}

HomeWindow::HomeWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::HomeWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Home");
}
