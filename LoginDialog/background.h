#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <QMainWindow>
#include <QDesktopWidget>
#include <QApplication>

#include "logindialog.h"

class background : public QMainWindow
{
    Q_OBJECT
public:
    explicit background(QWidget *parent = nullptr);

private:
    Logindialog *childWidget;

protected:
    void moveEvent(QMoveEvent *event) override;

signals:
    void moved();
};

#endif // BACKGROUND_H
