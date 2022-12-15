#include "progressbarwindow.h"
#include "ui_progressbarwindow.h"

ProgressbarWindow::ProgressbarWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressbarWindow)
{
    ui->setupUi(this);
}

ProgressbarWindow::~ProgressbarWindow()
{
    delete ui;
}
