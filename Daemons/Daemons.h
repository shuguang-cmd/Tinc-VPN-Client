#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDir>
#include <QTimer>
#include <string>
#include <QDebug>
#include "QtGui/private/qzipreader_p.h"
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QTextStream>

class Daemons : public QWidget
{
    Q_OBJECT

public:
    Daemons(QWidget *parent = 0);
    ~Daemons();


};

#endif // WIDGET_H
