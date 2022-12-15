#include "waitwindow.h"
#include "ui_waitwindow.h"

WaitWindow::WaitWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WaitWindow)
{
    ui->setupUi(this);
}

WaitWindow::~WaitWindow()
{
    delete ui;
}
