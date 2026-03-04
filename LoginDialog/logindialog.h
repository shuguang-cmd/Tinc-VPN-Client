#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QEventLoop>
#include <QObject>
#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QPainter>
#include <QPointer>

class confg;

class Logindialog : public QWidget
{
    Q_OBJECT

public:
    Logindialog(QWidget *parent = 0);
    ~Logindialog();

    void getBack(QNetworkReply *reply);
    void login();
    void cf();
    QString severIp_conf();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QLabel *Title;
    QLabel *Sid;
    QLabel *password;

    QLineEdit *uN;
    QLineEdit *pw;

    QPushButton *loginbtn;
    QPushButton *exitbtn;
    QEventLoop *m_loop;
    QString Token;
    QString sid;
    QString Password;
    QString Private;
    QByteArray PrivateArray;
    QNetworkAccessManager *manager;
    QString serverIp;
    QString parentpath;//路径
    QPointer<confg> m_confg;

signals:
    void closed();
};

#endif // LOGINDIALOG_H
