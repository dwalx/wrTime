#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <QtSql>
#include <QKeyEvent>
#include "timevalidator.h"

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
        TimeEntry()
        {
            isNull = false;
            id     = (quint64)-1;
        }
        TimeEntry(const TimeEntry &tm)
        {
            time1  = tm.time1;
            time2  = tm.time2;
            date   = tm.date;
            id     = tm.id;
            isNull = tm.isNull;
        }
    };


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
    QString inputTime(QString s = "", int min = 0);
    bool inputTimeEntry(TimeEntry *te, QString title = "");
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

    void on_tabWidget_currentChanged(int index);

    void on_btnTimeCreate_clicked();

    void on_btnTimeDelete_clicked();

    void on_pushButton_clicked();


protected:
    bool eventFilter(QObject* object, QEvent* event);
};

#endif // MAINWINDOW_H
