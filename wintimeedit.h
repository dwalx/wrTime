#ifndef WINTIMEEDIT_H
#define WINTIMEEDIT_H

#include <QDialog>

namespace Ui {
class WinTimeEdit;
}

class WinTimeEdit : public QDialog
{
    Q_OBJECT

public:
    explicit WinTimeEdit(QWidget *parent = 0);
    ~WinTimeEdit();

private:
    Ui::WinTimeEdit *ui;
};

#endif // WINTIMEEDIT_H
