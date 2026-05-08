#include "background.h"

background::background(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("T  M  G");
    setFixedSize(850, 600);

    // Create a central widget to hold everything and provide the background
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QDesktopWidget *desktop = QApplication::desktop();
    this->move((desktop->width() - this->width())/ 2, (desktop->height() - this->height()) /2);

    QString qss = R"(
        QWidget#centralWidget {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #667eea, stop:1 #764ba2);
        }
    )";
    centralWidget->setObjectName("centralWidget");
    centralWidget->setStyleSheet(qss);

    // Add decorative elements on the left side - parented to centralWidget
    QLabel *mainTitle = new QLabel("Secure Connect", centralWidget);
    mainTitle->setGeometry(60, 100, 400, 60);
    mainTitle->setStyleSheet("font-size: 48px; font-weight: bold; color: white; background: transparent;");

    QLabel *subTitle = new QLabel("Your Private Gateway to the World", centralWidget);
    subTitle->setGeometry(60, 160, 400, 30);
    subTitle->setStyleSheet("font-size: 18px; color: rgba(255, 255, 255, 0.8); background: transparent;");

    QString listStyle = "font-size: 14px; color: rgba(255, 255, 255, 0.7); background: transparent;";
    QLabel *feat1 = new QLabel("• High-speed encrypted tunnel", centralWidget);
    feat1->setGeometry(70, 240, 300, 25);
    feat1->setStyleSheet(listStyle);

    QLabel *feat2 = new QLabel("• Global network nodes support", centralWidget);
    feat2->setGeometry(70, 270, 300, 25);
    feat2->setStyleSheet(listStyle);

    QLabel *feat3 = new QLabel("• One-click intelligent routing", centralWidget);
    feat3->setGeometry(70, 300, 300, 25);
    feat3->setStyleSheet(listStyle);

    childWidget = new Logindialog(centralWidget);
    childWidget->setFixedSize(400, 500);
    childWidget->move(850 - 400 - 40, (600 - 500) / 2);
    childWidget->show();
}

void background::moveEvent(QMoveEvent *event)
{
    emit moved();
    QMainWindow::moveEvent(event);
}
