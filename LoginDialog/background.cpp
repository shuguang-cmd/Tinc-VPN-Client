#include "background.h"

background::background(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("T  M  G");
    setFixedSize(850, 600);

    QDesktopWidget *desktop = QApplication::desktop();
    this->move((desktop->width() - this->width())/ 2, (desktop->height() - this->height()) /2);

    QString qss = R"(
        QMainWindow {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #667eea, stop:1 #764ba2);
        }
    )";
    this->setStyleSheet(qss);

    // Add decorative elements on the left side
    QLabel *mainTitle = new QLabel("Secure Connect", this);
    mainTitle->setGeometry(60, 100, 400, 60);
    mainTitle->setStyleSheet("font-size: 48px; font-weight: bold; color: white; background: transparent;");

    QLabel *subTitle = new QLabel("Your Private Gateway to the World", this);
    subTitle->setGeometry(60, 160, 400, 30);
    subTitle->setStyleSheet("font-size: 18px; color: rgba(255, 255, 255, 0.8); background: transparent;");

    QString listStyle = "font-size: 14px; color: rgba(255, 255, 255, 0.7); background: transparent;";
    QLabel *feat1 = new QLabel("• High-speed encrypted tunnel", this);
    feat1->setGeometry(70, 240, 300, 25);
    feat1->setStyleSheet(listStyle);

    QLabel *feat2 = new QLabel("• Global network nodes support", this);
    feat2->setGeometry(70, 270, 300, 25);
    feat2->setStyleSheet(listStyle);

    QLabel *feat3 = new QLabel("• One-click intelligent routing", this);
    feat3->setGeometry(70, 300, 300, 25);
    feat3->setStyleSheet(listStyle);

    childWidget = new Logindialog(this);

    childWidget->setWindowFlags(childWidget->windowFlags() | Qt::Dialog);
    childWidget->setFixedSize(400, 500); // Consistent with Logindialog's size

    childWidget->show();

    connect(this, &background::moved, this, [this](){
            if (childWidget) {
                QPoint parentPos = this->pos();
                // Position on the right side (main width 850, child width 400, margin 40)
                QPoint newSwidgePos(parentPos.x() + (850 - 400 - 40), parentPos.y() + (600 - 500) / 2);
                childWidget->move(newSwidgePos);
            }
        });
}

void background::moveEvent(QMoveEvent *event)
{
    emit moved();
    QMainWindow::moveEvent(event);
}
