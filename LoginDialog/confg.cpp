#include "confg.h"
#include "logindialog.h"

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QFont>
#include <QUuid>
#include <QDebug>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTimer>

#include "download.h"


confg::confg(const QString& sid,const QString& Token,const QString& serverIp,QWidget *parent) :
    QWidget(parent),m_mainLayout(nullptr),m_download(nullptr)
{
    this->setFixedSize(650,500);
    this->setWindowTitle("Tinc内网配置向导");

    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);

    // 添加标题
    QLabel* titleLabel = new QLabel("Tinc内网配置向导", this);
    titleLabel->setFont(QFont("宋体", 14, QFont::Bold));
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 添加分割线
    QFrame* line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(line);

    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(20);
    buttonLayout->setContentsMargins(30, 10, 30, 10);

    startBtn = new QPushButton("开始配置", this);
    startBtn->setFont(QFont("宋体", 10));
    startBtn->setFixedSize(120, 35);
    startBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #4CAF50;"
        "   color: white;"
        "   border-radius: 4px;"
        "}"
        "QPushButton:hover { background-color: #45a049; }"
        );

    cancelBtn = new QPushButton("取消配置", this);
    cancelBtn->setFont(QFont("宋体", 10));
    cancelBtn->setFixedSize(120, 35);
    cancelBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #f44336;"
        "   color: white;"
        "   border-radius: 4px;"
        "}"
        "QPushButton:hover { background-color: #d32f2f; }"
        );

    buttonLayout->addWidget(startBtn);
    buttonLayout->addWidget(cancelBtn);
    mainLayout->addLayout(buttonLayout);

    // 连接信号槽
    connect(startBtn, &QPushButton::clicked, this, [=](){
        qDebug() << "开始配置按钮点击";
        this->dl(sid, Token, serverIp);
    });

    connect(cancelBtn, &QPushButton::clicked, this, [=](){
        QMessageBox msgBox(QMessageBox::Question, "提示", "确定取消配置?",
                           QMessageBox::Yes | QMessageBox::No, this);
        int result = msgBox.exec();
        if (result == QMessageBox::Yes) {
            this->cancel();
        }
    });

    qDebug() << "Token:" << Token;

    // QFont f0;
    // f0.setPointSize(14);
    // QFont f1;
    // f1.setPointSize(14);
    // QFont f2;
    // f2.setPointSize(14);

    // startBtn = new QPushButton;
    // startBtn->setText("开始配置");
    // startBtn->setFont(QFont("宋体",12));
    // startBtn->setGeometry(100,380,110,40);
    // startBtn->setParent(this);

    // cancelBtn = new QPushButton;
    // cancelBtn->setText("取消配置");
    // cancelBtn->setFont(QFont("宋体",12));
    // cancelBtn->setGeometry(435,380,110,40);
    // cancelBtn->setParent(this);

    // connect(startBtn,&QPushButton::clicked,this,[=](){
    //         this -> dl(sid,Token,serverIp);
    //     });

    // connect(cancelBtn,&QPushButton::clicked,this,[=](){
    //     QMessageBox msgBox(QMessageBox::Question, "提示", "确定取消配置?", QMessageBox::Yes | QMessageBox::No);
    //     int result = msgBox.exec();
    //     if (result == QMessageBox::Yes) {
    //         this->cancel();
    //     }
    // });
    // qDebug() <<Token;
}

void confg::cancel()
{
    Logindialog *father = new Logindialog;
    father->move(QApplication::desktop()->screen()->rect().center() - father->rect().center());
    father->show();
    this->close();
}

void confg::dl(QString sid,QString Token,QString serverIp)
{
    qDebug() << "开始创建 download 窗口";
    
    // 清理之前的 download 对象（如果存在）
    if (!m_download.isNull()) {
        qDebug() << "清理之前的 download 对象";
        m_download->close();
        m_download->deleteLater();
        m_download = nullptr;
        
        // 等待旧对象被销毁
        QEventLoop loop;
        QTimer::singleShot(100, &loop, &QEventLoop::quit);
        loop.exec();
    }
    
    // 创建下载界面，不设置parent以便独立显示
    // 使用成员变量 m_download，而不是局部变量
    m_download = new download(sid, Token, serverIp, nullptr);

    // 设置 download 对象的父对象为 nullptr，确保它不会被 confg 销毁
    m_download->setParent(nullptr);

    // 移除 WA_DeleteOnClose 属性，手动管理对象生命周期
    // m_download->setAttribute(Qt::WA_DeleteOnClose);

    // 设置下载界面位置居中
    m_download->move(QApplication::desktop()->screen()->rect().center() - m_download->rect().center());

    // 显示下载界面
    m_download->show();
    
    qDebug() << "download 窗口已显示";

    // 延迟关闭当前配置界面，确保 download 对象能够正常工作
    // 增加延迟时间到 2000ms，确保 download 对象完全初始化
    QTimer::singleShot(2000, [this]() {
        qDebug() << "准备关闭 confg 窗口";
        this->deleteLater();
        qDebug() << "confg 窗口已关闭";
    });
}

confg::~confg()
{
}
