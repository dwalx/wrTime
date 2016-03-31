#include "timevalidator.h"
#include <QDebug>
TimeValidator::TimeValidator(QObject* parent): QValidator(parent)
{
}

QValidator::State TimeValidator::validate(QString& input, int& pos ) const
{
    if (input.length() > 5) return QValidator::Invalid;
    QStringList digs = input.split(':');
    bool ok1, ok2;
    int dig1 = digs[0].toInt(&ok1);
    int dig2 = digs[1].toInt(&ok2);
    if (dig1 > 23 || dig2 > 59) return QValidator::Invalid;
    if (!ok1 && pos > 2) return QValidator::Invalid;
    if ((!ok2 || !ok1) && pos > 4) return QValidator::Invalid;
    return QValidator::Acceptable;
}
