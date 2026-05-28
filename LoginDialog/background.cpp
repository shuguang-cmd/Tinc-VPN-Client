#include "background.h"

background::background(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("Tinc VPN Terminal");
    setFixedSize(850, 600);

    QDesktopWidget *desktop = QApplication::desktop();
    this->move((desktop->width() - this->width())/ 2, (desktop->height() - this->height()) /2);

    // Main central widget to hold the left-side branding
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Left branding container
    QWidget *brandingArea = new QWidget(centralWidget);
    brandingArea->setGeometry(50, 100, 350, 400);
    
    QVBoxLayout *brandLayout = new QVBoxLayout(brandingArea);
    brandLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    brandLayout->setSpacing(10);

    QLabel *mainTitle = new QLabel("Tinc", brandingArea);
    mainTitle->setStyleSheet("font-size: 64px; font-weight: bold; color: white;");
    
    QLabel *subTitle = new QLabel("内网穿透终端", brandingArea);
    subTitle->setStyleSheet("font-size: 28px; color: rgba(255, 255, 255, 0.9); font-weight: 500;");

    QLabel *descLabel = new QLabel("安全 · 稳定 · 高效\n构建属于您的虚拟局域网", brandingArea);
    descLabel->setStyleSheet("font-size: 16px; color: rgba(255, 255, 255, 0.7); line-height: 1.5;");
    descLabel->setContentsMargins(0, 20, 0, 0);

    brandLayout->addWidget(mainTitle);
    brandLayout->addWidget(subTitle);
    brandLayout->addWidget(descLabel);

    QString qss = R"(
        QMainWindow {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, 
                stop:0 #667eea, stop:1 #764ba2);
        }
    )";
    this->setStyleSheet(qss);

    childWidget = new Logindialog(this);

    childWidget->setWindowFlags(childWidget->windowFlags() |Qt::Dialog);
    childWidget->setFixedSize(400, 500);

    childWidget->show();

    // Position the login dialog to the right
    connect(this, &background::moved, this, [this](){
            if (childWidget) {
                QPoint parentPos = this->pos();
                // Shifted to the right: using 80% of remaining width
                int xOffset = 420; 
                QPoint newSwidgePos(parentPos.x() + xOffset, 
                                    parentPos.y() + (this->height() - childWidget->height()) / 2);
                childWidget->move(newSwidgePos);
            }
        });
}

void background::moveEvent(QMoveEvent *event)
{
    emit moved();
    QMainWindow::moveEvent(event);
}
