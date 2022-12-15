#ifndef PROGRESSBARWINDOW_H
#define PROGRESSBARWINDOW_H

#include <QDialog>

namespace Ui {
class ProgressbarWindow;
}

class ProgressbarWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressbarWindow(QWidget *parent = nullptr);
    ~ProgressbarWindow();

private:
    Ui::ProgressbarWindow *ui;
};

#endif // PROGRESSBARWINDOW_H
