#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>

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
    };

    Ui::MainWindow *ui;
    QSqlDatabase db;
    QMap<QString, QString> settings;
    QList<TimeEntry> times;
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

    void setBtnSetTimeMode();
    int  iBtnSetTimeMode;
    const int btnSetTimeModeTime1 = 1;
    const int btnSetTimeModeTime2 = 2;

private slots:

    void on_btnSetTime_clicked();
};

#endif // MAINWINDOW_H
