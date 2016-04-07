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

    ui->lineEdit->setValidator(new TimeValidator(this));
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
    settings["sub_minutes"] = "-5";
    settings["add_minutes"] = "-5";
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
    QString today = QDate::currentDate().toString("yyyy.MM.dd");
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
    int row = ui->tblTime->currentRow();
    TimeEntry tm = times[row];
    if (inputTimeEntry(&tm))
    {
        times[row] = tm;
        ui->tblTime->item(row,0)->setText(tm.date);
        ui->tblTime->item(row,1)->setText(tm.time1);
        ui->tblTime->item(row,2)->setText(tm.time2);
        QString sql = QString("update wrTime set time1='%2', time2='%3', date='%4' where key = %1;").arg(times[row].id).arg(times[row].time1).arg(times[row].time2).arg(times[row].date);
        QSqlQuery q;
        q.exec(sql);
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
    QMessageBox* pmbx = new QMessageBox("Удаление",
                                        "Удалить выбранную запись о времени?",
                                        QMessageBox::Information,
                                        QMessageBox::NoButton,
                                        QMessageBox::Yes,
                                        QMessageBox::No);
    if (pmbx->exec() == QMessageBox::Yes) deleteRow(ui->tblTime->currentRow());
}

void MainWindow::deleteRow(int row)
{
    if (row < 0 || row > times.count()) return;
    QString sql = QString("delete from wrtime where key = %1;").arg(times[row].id);
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

void MainWindow::on_tabWidget_currentChanged(int index)
{
    if (index == 0) setBtnSetTimeMode();
}

void MainWindow::on_btnTimeCreate_clicked()
{

}

void MainWindow::on_btnTimeDelete_clicked()
{

}

bool MainWindow::inputTimeEntry(TimeEntry *te, QString title)
{
    bool res = false;
    if (!te) return res;

    QDialog *dlg = new QDialog(this);

    QVBoxLayout *layoutV = new QVBoxLayout(dlg);
    QHBoxLayout *layoutH = new QHBoxLayout(dlg);

    QPushButton *btnCancel = new QPushButton(dlg);
    QPushButton *btnOk     = new QPushButton(dlg);

    QDateEdit *date  = new QDateEdit(dlg);
    QTimeEdit *time1 = new QTimeEdit(dlg);
    QTimeEdit *time2 = new QTimeEdit(dlg);

    QLabel *lDate  = new QLabel(dlg);
    QLabel *lTime1 = new QLabel(dlg);
    QLabel *lTime2 = new QLabel(dlg);

    layoutV->setMargin(20);
    layoutV->addWidget(lDate);
    layoutV->addWidget(date);

    layoutV->addWidget(lTime1);
    layoutV->addWidget(time1);

    layoutV->addWidget(lTime2);
    layoutV->addWidget(time2);
    layoutV->addSpacing(10);

    layoutV->addLayout(layoutH);
    layoutH->setMargin(10);
    layoutH->addWidget(btnCancel);
    layoutH->addSpacing(30);
    layoutH->addWidget(btnOk);
    dlg->setLayout(layoutV);

    dlg->setWindowTitle(title != "" ? title : "Редактирование записи");
    btnCancel->setText("Отмена");
    btnOk->setText("Ок");
    date->setCalendarPopup(true);
    lDate->setText("Дата:");
    lTime1->setText("Время 1:");
    lTime2->setText("Время 2:");
    time1->setTime(QTime::fromString(te->time1,"hh:mm"));
    time2->setTime(QTime::fromString(te->time2,"hh:mm"));
    date->setDate(QDate::fromString(te->date,"yyyy.MM.dd"));

    connect(btnCancel, SIGNAL(clicked()), dlg, SLOT(reject()));
    connect(btnOk, SIGNAL(clicked()), dlg, SLOT(accept()));
    if (dlg->exec() == QDialog::Accepted)
    {
        te->date  = date->date().toString("yyyy.MM.dd");
        te->time1 = time1->time().toString("hh:mm");
        te->time2 = time2->time().toString("hh:mm");
        res = true;
    }

    delete dlg;
    return res;
}

QString MainWindow::inputTime(QString s, int min)
{
    QTime t = s == "" ? QTime::currentTime() : QTime::fromString(s, "hh:mm");
    t = t.addSecs(min * 60);
    s.sprintf("%02d:%02d",t.hour(),t.minute());

    Qt::WindowFlags flags = Qt::Dialog |  Qt::CustomizeWindowHint | Qt::WindowTitleHint |
                            Qt::WindowCloseButtonHint | Qt::WindowSystemMenuHint;
    QInputDialog *dlg = new QInputDialog(this, flags);

    QString ok(tr("Ok")),
            cancel("Отмена");
    dlg->setInputMode(QInputDialog::TextInput);
    dlg->setWindowTitle("Время");
    dlg->setLabelText("Введите время:");
    dlg->setOkButtonText(ok);
    dlg->setCancelButtonText(cancel);
    dlg->setTextValue(s);

    QLineEdit *le = dlg->findChild<QLineEdit*>();
    QList<QPushButton*> btns = dlg->findChildren<QPushButton*>();
    QPushButton *btn = NULL;
    for (auto b: btns)
        if (b->text() == ok) btn = b;

    auto disableButton = [btn] (QString str)
    {
        TimeValidator tm;
        int pos = 5;
        QValidator::State st = tm.validate(str, pos);
        btn->setEnabled(st == QValidator::Acceptable);
    };
    connect(le, &QLineEdit::textChanged, this, disableButton);

    le->setInputMask("09:09");
    le->setValidator(new TimeValidator(dlg));
    QString res = "";
    if (dlg->exec() == QDialog::Accepted) res = dlg->textValue();
    delete dlg;
    return res;
}

void MainWindow::on_pushButton_clicked()
{
    //inputTime("", settings["sub_minutes"].toInt());
    TimeEntry te;
    inputTimeEntry(&te);
}
