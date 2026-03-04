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
    setAttribute(Qt::WA_TranslucentBackground);  //设置窗口背景透明
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);

    QFont T_ft;
    T_ft.setFamily("楷体");
    T_ft.setPointSize(32);
    QFont ft;
    ft.setPointSize(17);
    ft.setFamily("STKaiti");
    QFont FT;
    FT.setPointSize(14);
    FT.setFamily("STKaiti");

    Title=new QLabel(QWidget::tr("Sign In"));
    Title->setGeometry(435,-75,300,250);
    Title->setFont(T_ft);
    Title->setStyleSheet("color:#696969;");
    Title->setParent(this);

    uN=new QLineEdit;
    uN->setGeometry(400,110,220,40);
    uN->setMaximumSize(220,40);
    uN->setPlaceholderText("UserName");
    uN->setMaxLength(12);
    uN->setClearButtonEnabled(true);
    uN->setParent(this);

    pw=new QLineEdit;
    pw->setGeometry(400,190,220,40);
    pw->setMaximumSize(220,40);
    pw->setPlaceholderText("PassWord");
    pw->setEchoMode(QLineEdit::Password);
    pw->setClearButtonEnabled(true);
    pw->setMaxLength(12);
    pw->setParent(this);

    loginbtn=new QPushButton;
    loginbtn->setText("sign in");
    loginbtn->setFont(QFont("仿宋",12));
    loginbtn->setGeometry(385,285,100,30);
    loginbtn->setFixedSize(85,30);
    loginbtn->setParent(this);
    loginbtn->setShortcut(Qt::Key_Return);//只能与大回车键相连，无法同时链接两个回车键

    exitbtn=new QPushButton;
    exitbtn->setText("sign out");
    exitbtn->setFont(QFont("仿宋",12));
    exitbtn->setGeometry(550,285,100,30);
    exitbtn->setFixedSize(85,30);
    exitbtn->setParent(this);

    this->setFixedSize(950,600);

    // QString programPath = QDir::currentPath();
    // QFileInfo fileInfo(programPath);
    // QDir parentDir = fileInfo.dir().path();
    // parentpath = parentDir.path();
    parentpath = "d:/Codes/Java/KenDeJi_RuoYi/tinc_cli_gui/windows";
    qDebug() << parentpath;

    connect(loginbtn,&QPushButton::clicked,this,&Logindialog::login);
    connect(exitbtn,&QPushButton::clicked,this,&Logindialog::close);
    manager = new QNetworkAccessManager(this);
    connect(manager,&QNetworkAccessManager::finished,this,&Logindialog::getBack);//通信完成后自动执行getBack
}

void Logindialog::paintEvent(QPaintEvent *event)
{
    QPainter painter(this); // 创建一个QPainter对象并指定绘制设备为this，即当前窗口
    painter.setRenderHint(QPainter::Antialiasing); // 设置绘制选项为反锯齿，使绘制的图形边缘更加平滑

    QColor color(0, 0, 0, 50);
    // 绘制窗口四周的阴影
    for(int i = 0; i < 10; i++)
    {
        // 1. 绘制阴影的路径
        QPainterPath path;
        // 2. 设置填充规则
        path.setFillRule(Qt::WindingFill);
        // 3. 添加一个圆角矩形
        path.addRoundedRect(QRect(10 - i, 10 - i, this->width() - (10 - i) * 2, this->height() - (10 - i) * 2), 15, 15);
        // 4. 设置颜色的透明度
        color.setAlpha(150 - qSqrt(i) * 45);
        painter.setPen(Qt::NoPen); // 不绘制边框线
        painter.setBrush(color); // 设置画刷颜色为阴影颜色
        // 5. 绘制阴影
        painter.drawPath(path);
    }

    QRectF rect(0, 0, 650, 400); // 获取当前窗口的矩形区域
    // 绘制整个窗口的圆角矩形，填充颜色为白色
    painter.setBrush(QBrush(QColor(245,245,245))); // 设置画刷颜色为灰色
    painter.setPen(Qt::transparent); // 设置画笔颜色为透明，即不绘制边框线
    painter.drawRoundedRect(rect, 15, 15); // 绘制一个带有圆角的矩形窗口，圆角半径为15px，如果把窗口设置成正方形，圆角半径设大，就会变成一个圆

    QPixmap pixmap(":/Icon/backgroud_Login.png");
    painter.drawPixmap(-7, -7, pixmap);

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

    Login_Api = QString("http://%1/XVntQFJCjc.php/myadmin/node/api").arg(serverIp);
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
        QNetworkRequest request;
        request.setUrl(Login_Api);
        QJsonObject json;
        json.insert("sid",sid);
        json.insert("password",Password);

        QJsonDocument document;
        document.setObject(json);//把json对象提取关键信息转换成数据流
        QByteArray dataArray = document.toJson(QJsonDocument::Compact);//把数据流转换成QByteArray变量，把JSon数据转换成Post参数
        request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");//设置请求头信息，向浏览器声明为Json数据

        manager->post(request,dataArray);//发出post请求
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
                    return;
                }

                QString sid = jsonObj.value("sid").toString();
                QString token = jsonObj.value("token").toString();
                QString netName = jsonObj.value("net_name").toString();

                QTextStream stream(&priFile);
                stream << "sid:" << sid << "\n";
                stream << "token:" << token << "\n";
                stream << "net_name:" << netName << "\n";
                stream << "server_ip:" << serverIp << "\n";

                qDebug() << "写入 private.txt 成功";
                qDebug() << "sid:" << sid;
                qDebug() << "token:" << token;
                qDebug() << "net_name:" << netName;

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
    }
    else
    {
        QString rt = QString::fromUtf8(PrivateArray);
        if(rt == "0")
            QMessageBox::critical(this,tr("错误"),tr("账号或密码错误"));
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


