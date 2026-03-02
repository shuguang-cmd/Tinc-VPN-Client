#ifndef CONFG_H
#define CONFG_H

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QVBoxLayout>

#include "logindialog.h"
#include "download.h"


class confg : public QWidget
{
    Q_OBJECT

public:
    confg(const QString& sid,const QString& Token,const QString& serverIp,QWidget *parent = 0);
    ~confg();

//    void cancel(QCloseEvent *event);
    void cancel();
    void dl(const QString sid,const QString Token,QString serverIp);

private:
    QPushButton *startBtn;
    QPushButton *cancelBtn;
    QVBoxLayout *m_mainLayout;
    download *m_download;

    QLabel *intr;
    QLabel *nod;
    QLabel *ip_add;

    QLineEdit *intranet;
    QLineEdit *node;
    QLineEdit *ip;

    QVBoxLayout *layout;

};

#endif // CONFG_H
