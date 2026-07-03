#include "logindialog.h"
#include "download.h"
#include "confg.h"

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QDialog>
#include <QMessageBox>
#include <QFile>
#include <QFont>
#include <QDebug>
#include <QKeySequence>
#include <QObject>
#include <QUrl>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QDir>
#include <QList>
#include <QPainterPath>

QString Login_Api;//登录Api

Logindialog::Logindialog(QWidget *parent)
    : QWidget(parent),m_confg(nullptr)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    this->setFixedSize(400, 500); // Adjusted size to match background.cpp
    setMouseTracking(true);

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    loginCard = new QWidget(this);
    loginCard->setObjectName("loginCard");
    loginCard->setFixedSize(360, 460); // Centered and taller
    loginCard->setMouseTracking(true);

    cardLayout = new QVBoxLayout(loginCard);
    cardLayout->setContentsMargins(40, 20, 40, 40);
    cardLayout->setSpacing(15);

    character = new AnimatedCharacter(loginCard);
    character->setMouseTracking(true);
    
    Title = new QLabel("VPN TERMINAL", loginCard);
    Title->setObjectName("loginTitle");
    Title->setAlignment(Qt::AlignCenter);
    Title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    Title->setMouseTracking(true);

    // Grid layout for input fields and labels
    QGridLayout *inputGrid = new QGridLayout();
    inputGrid->setSpacing(10);
    inputGrid->setContentsMargins(0, 0, 0, 0);

    Sid = new QLabel("节点名:", loginCard);
    Sid->setObjectName("loginLabel");
    uN = new QLineEdit(loginCard);
    uN->setObjectName("loginInput");
    uN->setPlaceholderText("请输入节点名...");
    uN->setClearButtonEnabled(true);
    uN->setMaxLength(12);
    uN->setMinimumHeight(45);
    uN->setMouseTracking(true);
    uN->installEventFilter(this);

    password = new QLabel("密  码:", loginCard);
    password->setObjectName("loginLabel");
    pw = new QLineEdit(loginCard);
    pw->setObjectName("loginInput");
    pw->setPlaceholderText("请输入密码...");
    pw->setEchoMode(QLineEdit::Password);
    pw->setClearButtonEnabled(true);
    pw->setMaxLength(64);
    pw->setMinimumHeight(45);
    pw->setMouseTracking(true);
    pw->installEventFilter(this);

    inputGrid->addWidget(Sid, 0, 0);
    inputGrid->addWidget(uN, 0, 1);
    inputGrid->addWidget(password, 1, 0);
    inputGrid->addWidget(pw, 1, 1);

    rememberCheck = new QCheckBox("记住节点名", loginCard);
    rememberCheck->setObjectName("rememberCheck");
    rememberCheck->setMouseTracking(true);

    // 从 QSettings 加载上次记住的节点名
    QSettings settings("TincVPN", "Client");
    if (settings.value("rememberNode", false).toBool()) {
        QString savedNode = settings.value("nodeName", "").toString();
        if (!savedNode.isEmpty()) {
            uN->setText(savedNode);
            rememberCheck->setChecked(true);
        }
    }

    buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(20);

    loginbtn = new QPushButton("登录", loginCard);
    loginbtn->setObjectName("loginButton");
    loginbtn->setMinimumSize(120, 40);
    loginbtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    loginbtn->setShortcut(Qt::Key_Return);

    exitbtn = new QPushButton("退出", loginCard);
    exitbtn->setObjectName("exitButton");
    exitbtn->setMinimumSize(120, 40);
    exitbtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    buttonLayout->addWidget(loginbtn);
    buttonLayout->addWidget(exitbtn);

    cardLayout->addWidget(character, 0, Qt::AlignCenter);
    cardLayout->addWidget(Title);
    cardLayout->addLayout(inputGrid);
    cardLayout->addWidget(rememberCheck, 0, Qt::AlignLeft);
    cardLayout->addLayout(buttonLayout);
    cardLayout->addStretch();

    mainLayout->addWidget(loginCard, 0, Qt::AlignCenter);

    // 动态计算路径，自适应寻找包含关键配置或目录的根目录
    QDir searchDir(QCoreApplication::applicationDirPath());
    bool foundRoot = false;
    for (int i = 0; i < 5; ++i) {
        // 使用只读的 Tinc/tincd.exe 和 serverIp.conf 进行判定，避免被残留的 private.txt 污染干扰
        if (QFile::exists(searchDir.absoluteFilePath("Tinc/tincd.exe")) || 
            QFile::exists(searchDir.absoluteFilePath("serverIp.conf"))) {
            parentpath = searchDir.absolutePath();
            foundRoot = true;
            break;
        }
        if (!searchDir.cdUp()) break;
    }
    if (!foundRoot) {
        // 回退逻辑
        QDir fallback(QCoreApplication::applicationDirPath());
        fallback.cdUp();
        parentpath = fallback.absolutePath();
    }
    qDebug() << "Logindialog 根目录:" << parentpath;

    connect(loginbtn, &QPushButton::clicked, this, &Logindialog::login);
    connect(exitbtn, &QPushButton::clicked, this, &Logindialog::close);
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &Logindialog::getBack);

    QString qss = R"(
        Logindialog {
            background: transparent;
        }
        
        QWidget#loginCard {
            background-color: white;
            border-radius: 15px;
        }
        
        QLabel#loginTitle {
            font-size: 24px;
            font-weight: bold;
            color: #333333;
            padding: 10px;
        }

        QLabel#loginLabel {
            font-size: 14px;
            color: #555555;
            font-weight: bold;
            min-width: 60px;
        }
        
        QLineEdit#loginInput {
            background-color: #f5f5f5;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            padding: 12px;
            font-size: 14px;
            color: #333333;
        }
        
        QLineEdit#loginInput:focus {
            border: 2px solid #667eea;
            background-color: white;
        }
        
        QPushButton#loginButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                stop:0 #667eea, stop:1 #764ba2);
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            font-weight: bold;
            padding: 10px;
        }
        
        QPushButton#loginButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                stop:0 #5568d3, stop:1 #6388b8);
        }
        
        QPushButton#loginButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                stop:0 #4456a2, stop:1 #517a7a);
        }
        
        QPushButton#exitButton {
            background-color: #f5f5f5;
            color: #666666;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            font-size: 14px;
            font-weight: bold;
            padding: 10px;
        }
        
        QPushButton#exitButton:hover {
            background-color: #e0e0e0;
            color: #333333;
        }
        
        QPushButton#exitButton:pressed {
            background-color: #d0d0d0;
        }

        QCheckBox#rememberCheck {
            color: #666666;
            font-size: 12px;
            spacing: 6px;
        }
    )";
    this->setStyleSheet(qss);
}

void Logindialog::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Use a soft blue-ish shadow instead of black
    QColor shadowColor(102, 126, 234, 15); 
    
    for(int i = 0; i < 15; i++)
    {
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRoundedRect(QRect(15 - i, 15 - i, this->width() - (15 - i) * 2, this->height() - (15 - i) * 2), 15, 15);
        shadowColor.setAlpha(40 - i * 2);
        painter.setPen(Qt::NoPen);
        painter.setBrush(shadowColor);
        painter.drawPath(path);
    }
}

QString Logindialog::severIp_conf()
{
    QFile file(parentpath + "/serverIp.conf");
    QString ipAddress;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug()<<"文件打开失败";
    }

    // 读取文件内容
    QString content = file.readAll();

    // 获取server_ip字段的值
    QStringList lines = content.split("\n");
    qDebug()<< lines;
    for (const QString& line : lines) {
        int firstColon = line.indexOf(':');
        if (firstColon != -1) {
            QString key = line.left(firstColon).trimmed();
            if (key == "server_ip") {
                ipAddress = line.mid(firstColon + 1).trimmed();
                break;
            }
        }
    }
    return ipAddress;
}

void Logindialog::login()
{
    serverIp = severIp_conf();
    qDebug()<<serverIp;

    QString protocol = (serverIp.startsWith("127.0.0.1") || serverIp.startsWith("localhost") || serverIp.startsWith("192.168.") || serverIp.startsWith("10.")) ? "http" : "https";
    Login_Api = QString("%1://%2/api/tinc/client/login").arg(protocol).arg(serverIp);
    qDebug()<<Login_Api;

    sid = this->uN->text().remove(QChar('\040'));
    Password = this->pw->text().remove(QChar('\040'));

    if(0 == sid.size() || Password.size()==0)
    {
         QMessageBox *infoBox = new QMessageBox;
         infoBox -> critical(this,tr("error"),tr("用户名或密码不能为空！"));
         this->pw->setFocus();
    }
    else
    {
        // 保存"记住节点名"设置
        QSettings settings("TincVPN", "Client");
        settings.setValue("rememberNode", rememberCheck->isChecked());
        if (rememberCheck->isChecked()) {
            settings.setValue("nodeName", sid);
        } else {
            settings.remove("nodeName");
        }

        // 登录中状态：禁用按钮，防止重复点击
        loginbtn->setEnabled(false);
        loginbtn->setText("登录中...");

        QNetworkRequest request;
        request.setUrl(Login_Api);
        QJsonObject json;
        json.insert("sid",sid);
        json.insert("password",Password);

        QJsonDocument document;
        document.setObject(json);
        QByteArray dataArray = document.toJson(QJsonDocument::Compact);
        request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

        manager->post(request,dataArray);
    }

}

QString removeSymbols(QString str, QChar symbol) {
    str = str.trimmed(); // 去除字符串两端的空格

    if (str.startsWith(symbol)) {
        str = str.mid(1); // 删除开头的字符
    }

    if (str.endsWith(symbol)) {
        str = str.left(str.length() - 1); // 删除结尾的字符
    }

    return str;
}

void Logindialog::getBack(QNetworkReply *reply)
{

    //生成private.txt文件
    QFile priFile(parentpath + "/private.txt");
    if(parentpath.isEmpty() == false)
    {
        //打开文件，只写
        if(priFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))  // 关键点2：修改打开模式
        {
            PrivateArray = reply->readAll();

            QJsonParseError parseError;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(PrivateArray, &parseError);

            if(parseError.error == QJsonParseError::NoError)
            {
                QJsonObject jsonObj = jsonDoc.object();

                int status = jsonObj.value("status").toInt();
                QString msg = jsonObj.value("msg").toString();

                if(status == 0)
                {
                    qDebug() << "登录失败：" << msg;
                    QMessageBox::critical(this,tr("错误"),tr(msg.toUtf8()));
                    priFile.close();
                    loginbtn->setEnabled(true);
                    loginbtn->setText("登录");
                    return;
                }

                // 👇 提取 JSON 里的数据（重点增加 node_ip）
                QString sid = jsonObj.value("sid").toString();
                QString token = jsonObj.value("token").toString();
                QString netName = jsonObj.value("net_name").toString();
                QString nodeIp = jsonObj.value("node_ip").toString(); // 👈 新增：提取分配的虚拟 IP

                QTextStream stream(&priFile);
                stream << "sid:" << sid << "\n";
                stream << "token:" << token << "\n";
                stream << "net_name:" << netName << "\n";
                stream << "server_ip:" << serverIp << "\n";
                stream << "node_ip:" << nodeIp << "\n"; // 👈 新增：写入 private.txt

                qDebug() << "写入 private.txt 成功";
                qDebug() << "sid:" << sid;
                qDebug() << "token:" << token;
                qDebug() << "net_name:" << netName;
                qDebug() << "server_ip:" << serverIp;
                qDebug() << "node_ip:" << nodeIp; // 👈 打印出来确认一下！

                if(stream.status() != QTextStream::Ok)
                {
                    qDebug() << "写入文件时发生错误";
                }
            }
            else
            {
                qDebug() << "JSON解析失败：" << parseError.errorString();
            }

            priFile.close();
        }
        else
        {
            qDebug() << "无法打开文件：" << priFile.errorString();  // 关键点5：输出错误信息
            qDebug() << "文件路径：" << priFile.fileName();
        }
    }
    else
    {
        qDebug() << "父路径为空，无法创建文件";
    }
        priFile.close();

    //获取http状态码
    QVariant statusCode = reply ->attribute(QNetworkRequest::HttpReasonPhraseAttribute);
    if(statusCode.isValid())
        qDebug() << "status code=" << statusCode.toInt();

    QVariant reason = reply -> attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    if(reason.isValid())
        qDebug() << "reason=" << reason.toString();

    QNetworkReply::NetworkError err = reply -> error();
    if(err != QNetworkReply::NoError)
    {
        QMessageBox::critical(this,tr("错误"),tr(reply->readAll()));
        loginbtn->setEnabled(true);
        loginbtn->setText("登录");
    }
    else
    {
        QString rt = QString::fromUtf8(PrivateArray);
        if(rt == "0") {
            QMessageBox::critical(this,tr("错误"),tr("账号或密码错误"));
            loginbtn->setEnabled(true);
            loginbtn->setText("登录");
        }
        else {
            QString line;
            if(priFile.open(QIODevice::ReadOnly))
            {
                QTextStream stream(&priFile);
                while(!stream.atEnd())
                {
                    line = stream.readLine();
                    if(line.contains("token"))
                    {

                        QStringList linelist = line.split(":");
                        qDebug()<<linelist[1];
                        Token = QString(linelist[1]);
                        Token = removeSymbols(Token,'\"');
                        Token = removeSymbols(Token,',');
                        Token = removeSymbols(Token,'\"');
                        break;
                    }
                }
                priFile.close();

            }
            qDebug() << Token;
            emit closed();
            this->close();
            
            // 清理之前的 confg 对象（如果存在）
            if (!m_confg.isNull()) {
                qDebug() << "清理之前的 confg 对象";
                m_confg->close();
                m_confg->deleteLater();
                m_confg = nullptr;
                
                // 等待旧对象被销毁
                QEventLoop loop;
                QTimer::singleShot(100, &loop, &QEventLoop::quit);
                loop.exec();
            }
            
            // 创建 confg 对象，使用成员变量以便管理生命周期
            m_confg = new confg(sid,Token,serverIp);
            QRect availableGeometry = QApplication::desktop()->availableGeometry();
            int x = (availableGeometry.width() - m_confg->width()) / 2;
            int y = (availableGeometry.height() - m_confg->height()) / 2;
            m_confg->move(x, y);
            m_confg->show();
            
            qDebug() << "confg 窗口已创建并显示";
            
            // 设置 confg 对象的父对象为 nullptr，确保它不会被 logindialog 销毁
            m_confg->setParent(nullptr);
        }
    }
}

Logindialog::~Logindialog()
{

}

void Logindialog::mousePressEvent(QMouseEvent *event)
{
    // Dragging disabled to keep it fixed to the main frame
    QWidget::mousePressEvent(event);
}

void Logindialog::mouseMoveEvent(QMouseEvent *event)
{
    // Still update eye tracking
    if (character) {
        character->setTargetPos(event->globalPos());
    }
    
    // Dragging logic removed
    QWidget::mouseMoveEvent(event);
}

bool Logindialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == pw) {
        if (event->type() == QEvent::FocusIn) {
            character->setCoverEyes(true);
        } else if (event->type() == QEvent::FocusOut) {
            character->setCoverEyes(false);
        }
    }
    return QWidget::eventFilter(obj, event);
}


