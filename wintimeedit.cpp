#include "wintimeedit.h"
#include "ui_wintimeedit.h"

WinTimeEdit::WinTimeEdit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WinTimeEdit)
{
    ui->setupUi(this);
}

WinTimeEdit::~WinTimeEdit()
{
    delete ui;
}
