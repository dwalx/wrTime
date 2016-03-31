#ifndef TIMEVALIDATOR_H
#define TIMEVALIDATOR_H

#include <QObject>
#include <QValidator>

class TimeValidator: public QValidator
{
    //Q_OBJECT
public:
   TimeValidator(QObject* parent = 0);
   virtual QValidator::State validate(QString& input, int& pos ) const;


signals:

public slots:
};

#endif // TIMEVALIDATOR_H
