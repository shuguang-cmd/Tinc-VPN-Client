#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QEventLoop>
#include <QObject>
#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QPainter>
#include <QPointer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QSettings>
#include "AnimatedCharacter.h"

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
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    AnimatedCharacter *character;
    QLabel *Title;
    QLabel *Sid;
    QLabel *password;

    QLineEdit *uN;
    QLineEdit *pw;

    QPushButton *loginbtn;
    QPushButton *exitbtn;
    QCheckBox *rememberCheck;
    QEventLoop *m_loop;
    QString Token;
    QString sid;
    QString Password;
    QString Private;
    QByteArray PrivateArray;
    QNetworkAccessManager *manager;
    QString serverIp;
    QString parentpath;
    QPointer<confg> m_confg;
    
    QWidget *loginCard;
    QVBoxLayout *mainLayout;
    QVBoxLayout *cardLayout;
    QHBoxLayout *buttonLayout;
    QGraphicsDropShadowEffect *shadowEffect;
    
    QPoint m_dragPosition;

signals:
    void closed();
};

#endif // LOGINDIALOG_H
