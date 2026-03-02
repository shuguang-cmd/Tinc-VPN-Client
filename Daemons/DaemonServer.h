#ifndef SERVER_H
#define SERVER_H

#include "qtservice.h"
//#define WIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDir>
#include <QTimer>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QTextStream>
#include <QRegularExpression>

class DaemonService :public QtService<QCoreApplication>,public QObject
{
public:
    DaemonService(int argc, char **argv);

    QTcpServer* server;
    QFile mainFile;

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

    QString getMess(QString message)
    {
        // QString programPath = QDir::currentPath();
        // QFileInfo fileInfo(programPath);
        // QDir parentDir = fileInfo.dir().path();
        // QString parentpath = parentDir.path();
        // qDebug() << parentpath;

        QString parentpath = "D:/Users/laboratory/client_local/windows";
        qDebug() << parentpath;

        QFile priFile(parentpath + "/private.txt");
        QString line;
        QString out;

        if(priFile.open(QIODevice::ReadOnly))
        {
            QTextStream stream(&priFile);
            while(!stream.atEnd())
            {
                line = stream.readLine();
                if(line.contains(message))
                {

                    QStringList linelist = line.split(":");
                    qDebug()<<linelist[1];
                    out = QString(linelist[1]);
                    out = removeSymbols(out,'\"');
                    out = removeSymbols(out,',');
                    out = removeSymbols(out,'\"');
                    break;
                }
            }
            priFile.close();
        }

        return out;
    }

    void setTime();
    void getIP();
    void HeartReply(QNetworkReply *reply);
    int checkServiceStatus();
    bool modifyFileContent(QString filePath, QString targetKey, QString newValue);
    bool modifyPrivateFile(QString targetKey, QString newValue);
    void service();
    void successReply(QString details,QString key,QString value);
    void failureReply(QString details);
    bool conf_pack(QString netname,QString nodeip);
    bool deleteFile(QString path,QString fileName);
    QString getLatestServiceName();
    void kill_conf(QString processName);
    QString severIp_conf()
    {
        // QString programPath = QDir::currentPath();
        // QFileInfo fileInfo(programPath);
        // QDir parentDir = fileInfo.dir().path();
        // QString path = parentDir.path();

        QString path = "d:/Codes/Java/KenDeJi_RuoYi/tinc_cli_gui/windows";
        qDebug() << path;
        QString ipAddress;

        QFile file(path + "/serverIp.conf");
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
    };

    QNetworkAccessManager* heartbeatReq;
    QNetworkAccessManager* mainReq;
    QNetworkAccessManager* nodeReq;
    QNetworkAccessManager* editReply;

private:
    const wchar_t* serviceName;

    QString servicename;//服务全称
    QString netName;//内网名称
    QString parentDir;//上一级目录
    QString serverIp ;//接入服务器地址
    QString gatewayIp;//网关地址
    QString sid;//设备ID
    QString id;//数据库序号
    QString old_nodeName;//旧节点名称
    QString nodeName;//当前节点名称
    QString old_nodeIp;//旧节点ip
    QString nodeIp;//当前节点ip
    QString action;//操作类型
    QString Port;
    QTimer *timer;//心跳信号
    QTimer *timer0;//ping
    QTimer *timer1;//checkservicestatu


public slots:
    void sendHeartBeat();
    void ping();

protected:
    void start()override;
    void stop()override;
    void pause()override{}
    void resume()override{}
};

#endif // SERVER_H
