#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDate>
#include <QTime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tblTime->installEventFilter(this);

    frmTimeEdit = new WinTimeEdit(this);

    QDate date = QDate::currentDate();
    ui->cbMonth->setCurrentIndex(date.month()-1);
    ui->sbYear->setValue(date.year());
    QString str;
    str.sprintf("%02d.%02d.%02d", date.day(), date.month(), date.year()-2000);
    ui->lcdDate->display(str);
    dbOpen();
    loadSettings();

    int m = calcTimeMonth(date.month(), date.year());
    ui->lcdMinutes->display(QString::number(m));
    QPalette pal;
    pal.setColor(QPalette::WindowText, m >= 0 ? Qt::green : Qt::red);
    ui->lcdMinutes->setPalette(pal);

    setBtnSetTimeMode();

    loadTimeMonth(times, date.month(), date.year());
    fillTable();    
}

int  MainWindow::calcTimeMonth(int month, int year)
{
    QList<TimeEntry> tm;
    QSqlQuery q;
    QString d1, d2;
    d1.sprintf("%04d.%02d.01",year,month);
    QDate d(year, month, 1);
    d2.sprintf("%04d.%02d.%02d", year, month, d.daysInMonth());
    QString sql = QString("select distinct date from wrtime where date >= '%1' and date <= '%2';").arg(d1).arg(d2);
    q.exec(sql);
    int cnt = 0;
    while (q.next()) cnt++;
    loadTimeMonth(tm, month, year);
    int m = calcWorkMinutes(tm)-cnt*settings["wday_len"].toInt();
    if (m < 0) m = 0;
    return m;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    dbClose();
}

bool MainWindow::dbOpen()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    QString db_name = QApplication::applicationDirPath()+"/wrtime.db3";
    db.setDatabaseName(db_name);
    if (!db.open()) return false;
    if (!db.tables().contains("wrtime", Qt::CaseInsensitive))
    {
        QSqlQuery q;
        QString sql = "create table wrtime(key integer primary key,"
                                           "date char(24),  "
                                           "time1 char(24), "
                                           "time2 char(24));";
        if (!q.exec(sql)) return false;
    }
    if (!db.tables().contains("wrsettings", Qt::CaseInsensitive))
    {
        QSqlQuery q;
        QString sql = "create table wrsettings(key integer primary key,"
                                              "name  char(32),  "
                                              "value char(128)); ";
        if (!q.exec(sql)) return false;
    }
    return true;
}

void MainWindow::resetSettings()
{
    settings["wday_begin" ] = "08:00";
    settings["wday_end"   ] = "17:00";
    settings["lunch_begin"] = "12:00";
    settings["lunch_end"  ] = "13:00";
    calcSettingsValues();
}

void MainWindow::calcSettingsValues()
{
    QTime wday_begin  = QTime::fromString(settings["wday_begin" ], "hh:mm");
    QTime wday_end    = QTime::fromString(settings["wday_end"   ], "hh:mm");
    QTime lunch_begin = QTime::fromString(settings["lunch_begin"], "hh:mm");
    QTime lunch_end   = QTime::fromString(settings["lunch_end"  ], "hh:mm");
    settings["lunch_len"   ] = QString::number(lunch_begin.msecsTo(lunch_end) / (60*1000));
    settings["lunch_len_ms"] = QString::number(lunch_begin.msecsTo(lunch_end));
    settings["wday_len_ms" ] = QString::number((wday_begin.msecsTo(wday_end)
                                               - settings["lunch_len_ms"].toInt()) );
    settings["wday_len"    ] = QString::number(settings["wday_len_ms"].toInt() / (60*1000));
}

void MainWindow::loadSettings()
{
    resetSettings();
    QSqlQuery q("select name, value from wrsettings;");
    while (q.next())
        settings[q.value("name").toString()] = q.value("value").toString();
    calcSettingsValues();
}

void MainWindow::saveSettings()
{
    QSqlQuery q;    
    for (auto it = settings.begin(); it != settings.end(); ++it)
    {
        QString sql;
        sql = QString("select count(name) from wrsettings where name='%1';").arg(it.key());
        q.exec(sql);
        q.next();
        if (!q.value(0).toInt())
            sql = QString("insert into wrsettings(name, value) values('%1', '%2');").arg(it.key()).arg(it.value());
        else
            sql = QString("update wrsettings set value = '%1' where name = '%2';").arg(it.value()).arg(it.key());
        q.exec(sql);        
    }
}

void MainWindow::dbClose()
{
    if (db.isOpen()) db.close();
}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::calcWorkMinutes(QList<TimeEntry> &tm)
{
    if (tm.isEmpty()) return 0;
    int ms = 0;
    QTime lunch_begin = QTime::fromString(settings["lunch_begin"], "hh:mm");
    QTime lunch_end   = QTime::fromString(settings["lunch_end"  ], "hh:mm");        
    for (auto it: tm)
    {
        if (it.isNull) continue;
        QTime t1 = QTime::fromString(it.time1, "hh:mm");
        QTime t2 = QTime::fromString(it.time2, "hh:mm");
        if (t1 >= lunch_begin && t1 <= lunch_end) t1 = lunch_end;
        if (t2 >= lunch_begin && t2 <= lunch_end) t2 = lunch_begin;
        int tm_ms = t1.msecsTo(t2);
        if (tm_ms < 0) tm_ms = 0;
        if (t1 < lunch_begin && t2 > lunch_end) tm_ms -= settings["lunch_len_ms"].toInt();        
        ms += tm_ms;
    }
    return ms / (60*1000);
}

void MainWindow::loadTimeMonth(QList<TimeEntry> &tm, int month, int year)
{
   QString d1, d2;
   d1.sprintf("%04d.%02d.%02d", year, month, 1);
   QDate d(year, month, 1);
   d2.sprintf("%04d.%02d.%02d", year, month, d.daysInMonth());

   QSqlQuery q;   
   QString sql = QString("select key, date, time1, time2 from wrtime where date >= '%1' and date <= '%2';").arg(d1).arg(d2);
   q.exec(sql);
   while (q.next())
   {
        TimeEntry t;
        t.isNull = q.value("time1").isNull() || q.value("time2").isNull() || q.value("date").isNull();
        t.time1  = q.value("time1").toString();
        t.time2  = q.value("time2").toString();
        t.date   = q.value("date").toString();
        t.id     = q.value("key").toULongLong();
        tm << t;
   }
}

void MainWindow::fillTable()
{
    ui->tblTime->clear();
    ui->tblTime->setColumnCount(3);
    ui->tblTime->setRowCount(times.count());
    ui->tblTime->setHorizontalHeaderItem(0, new QTableWidgetItem("Дата"));
    ui->tblTime->setHorizontalHeaderItem(1, new QTableWidgetItem("Время 1"));
    ui->tblTime->setHorizontalHeaderItem(2, new QTableWidgetItem("Время 2"));
    int row = 0;    
    for (auto it: times)
    {
        ui->tblTime->setItem(row, 0, new QTableWidgetItem(it.date));
        ui->tblTime->setItem(row, 1, new QTableWidgetItem(it.time1));
        ui->tblTime->setItem(row, 2, new QTableWidgetItem(it.time2));
        row++;
    }
    emit on_tblTime_itemSelectionChanged();
}

void MainWindow::setBtnSetTimeMode()
{
    QString today = QDate::currentDate().toString("yyyy.MM.dd");;
    QSqlQuery q;
    QString sql = QString("select key, time1, time2 from wrtime where date = '%1' order by time1 desc limit 1;").arg(today);
    q.exec(sql);
    if (q.next())
    {
        if (q.value("time2").isNull()) iBtnSetTimeMode = btnSetTimeModeTime2;        
        u64Time2Id = q.value("key").toULongLong();
    }
    else iBtnSetTimeMode = btnSetTimeModeTime1;
    switch (iBtnSetTimeMode)
    {
        case btnSetTimeModeTime1: ui->btnSetTime->setText("Добавить"); break;
        case btnSetTimeModeTime2: ui->btnSetTime->setText("Закрыть"); break;
    }
}

void MainWindow::on_btnSetTime_clicked()
{

    switch (iBtnSetTimeMode)
    {
        case btnSetTimeModeTime1: break;
        case btnSetTimeModeTime2: break;
    }
    setBtnSetTimeMode();
}

void MainWindow::on_btnTimeEdit_clicked()
{
    frmTimeEdit->setModal(true);
    int row = ui->tblTime->currentRow();
    frmTimeEdit->date  = times[row].date;
    frmTimeEdit->time1 = times[row].time1;
    frmTimeEdit->time2 = times[row].time2;
    frmTimeEdit->exec();
    if (frmTimeEdit->result())
    {
        times[row].date  = frmTimeEdit->date;
        times[row].time1 = frmTimeEdit->time1;
        times[row].time2 = frmTimeEdit->time2;
        QString sql = QString("update wrtime set date='%1', time1='%2', time2='%3' where key=%4").arg(times[row].date).arg(times[row].time1).arg(times[row].time2).arg(times[row].id);
        QSqlQuery q;
        q.exec(sql);
        ui->tblTime->item(row,0)->setText(times[row].date);
        ui->tblTime->item(row,1)->setText(times[row].time1);
        ui->tblTime->item(row,2)->setText(times[row].time2);
    }
}

bool MainWindow::eventFilter(QObject* object, QEvent* event)
{
    bool handled = false;
    if (object == ui->tblTime && event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        bool flg = false;
        switch (keyEvent->key())
        {
        case Qt::Key_Delete: emit deleteSelectedRow(); handled = true; break;
        case Qt::Key_Insert: break;
        }

    }
    return handled ? true : QWidget::eventFilter(object, event);
}

void MainWindow::deleteSelectedRow()
{
    deleteRow(ui->tblTime->currentRow());
}

void MainWindow::deleteRow(int row)
{
    if (row < 0 || row > times.count()) return;
    QString sql = QString("delete from wrtime where key = %1").arg(times[row].id);
    QSqlQuery q;
    q.exec(sql);
    ui->tblTime->removeRow(row);
    times.removeAt(row);
}

void MainWindow::on_tblTime_itemSelectionChanged()
{
    bool validSelection = ui->tblTime->currentRow() >= 0 &&
                          ui->tblTime->currentRow() <  ui->tblTime->rowCount();
    ui->btnTimeDelete->setEnabled(validSelection);
    ui->btnTimeEdit->setEnabled(validSelection);
}
