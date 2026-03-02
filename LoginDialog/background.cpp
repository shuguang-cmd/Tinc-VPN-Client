#include "background.h"

background::background(QWidget *parent) : QMainWindow(parent)
{
    //开启背景设置
    this->setAutoFillBackground(true);
    //创建调色板对象
    QPalette p = this->palette();
    //加载图片
    QPixmap pix(":/Icon/VCG41N846014486.jpg");
    //设置图片
    p.setBrush(QPalette::Window,QBrush(pix));
    this->setPalette(p);

    setWindowTitle("T  M  G");
    setFixedSize(850, 600);

    QDesktopWidget *desktop = QApplication::desktop();
    this->move((desktop->width() - this->width())/ 2, (desktop->height() - this->height()) /2);

    // 创建子窗口实例
    childWidget = new Logindialog(this);


    //将子窗口置于顶层
    childWidget->setWindowFlags(childWidget->windowFlags() |Qt::Dialog);
    childWidget->setFixedSize(657,407);
//    childWidget->setFixedSize(650,400);
    childWidget->show();

    connect(this, &background::moved, this, [this](){
            if (childWidget) {
                QPoint parentPos = this->pos();
                QPoint newSwidgePos(parentPos.x() + 95, parentPos.y() + 140);  // 让子窗口距离父窗口左上角固定偏移
                childWidget->move(newSwidgePos);
            }
        });

    connect(childWidget, SIGNAL(closed()), this, SLOT(close()));
}

void background::moveEvent(QMoveEvent *event)
{
    emit moved();
    QMainWindow::moveEvent(event);
}
