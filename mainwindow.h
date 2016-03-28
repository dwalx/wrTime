#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QKeyEvent>
#include "wintimeedit.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    struct TimeEntry
    {
        QString time1;
        QString time2;
        QString date;
        quint64 id;
        bool    isNull;
    };

    WinTimeEdit* frmTimeEdit;

    Ui::MainWindow *ui;
    QSqlDatabase db;
    QMap<QString, QString> settings;
    QList<TimeEntry> times;
    int  selectedRow;
    bool dbOpen();
    void dbClose();
    void loadSettings();
    void saveSettings();
    void resetSettings();
    void calcSettingsValues();
    int  calcWorkMinutes(QList<TimeEntry> &tm);
    int  calcTimeMonth(int month, int year);
    void closeEvent(QCloseEvent *event);
    void loadTimeMonth(QList<TimeEntry> &tm, int month, int year);
    void fillTable();

    void setBtnSetTimeMode();
    int  iBtnSetTimeMode;
    quint64 u64Time2Id;
    static const int btnSetTimeModeTime1 = 1;
    static const int btnSetTimeModeTime2 = 2;

private slots:

    void on_btnSetTime_clicked();
    void on_btnTimeEdit_clicked();


    void deleteRow(int row);
    void deleteSelectedRow();

    void on_tblTime_itemSelectionChanged();

protected:
    bool eventFilter(QObject* object, QEvent* event);
};

#endif // MAINWINDOW_H
