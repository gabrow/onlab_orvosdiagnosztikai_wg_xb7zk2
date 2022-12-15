#ifndef WAITWINDOW_H
#define WAITWINDOW_H

#include <QDialog>

namespace Ui {
class WaitWindow;
}

class WaitWindow : public QDialog
{
    Q_OBJECT

public:
    explicit WaitWindow(QWidget *parent = nullptr);
    ~WaitWindow();

private:
    Ui::WaitWindow *ui;
};

#endif // WAITWINDOW_H
