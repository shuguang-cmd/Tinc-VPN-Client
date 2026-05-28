#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QPointer>
#include <QFile>
#include <QProcess>
#include <QFileDialog>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QString>
#include <QThread>
#include <QMessageBox>
#include <QProgressDialog>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QTimer>

//extern QStringList InterList;//??????????
//extern QStringList InterList_new;//??????????



class download : public QWidget
{
    Q_OBJECT

public:
    static QProcess* dldprocess;

public:
    explicit download(const QString& sid,const QString& Token,const QString& serverIp,QWidget *parent = nullptr);
    ~download();

    void return_clicked(QNetworkReply *reply);
    void text(QNetworkReply *reply);
    void Decompression();
    void cmdProcess();
    void setThread();
    void serviceThread();
    void test();
    void conf_pack();
    void service();
    void Daemon();
    QString removeSymbols2(QString str, QChar symbol) {
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
        // 动态计算路径，向上跳三级到达 code_win 目录
        QDir appDir(QCoreApplication::applicationDirPath());
        appDir.cdUp(); // build
        appDir.cdUp(); // LoginDialog
        appDir.cdUp(); // code_win
        
        QString parentpath = appDir.absolutePath();
        qDebug() << "getMess 根目录:" << parentpath;

        QFile priFile(parentpath + "/private.txt");
        QString line;
        QString out;

        if(priFile.open(QIODevice::ReadOnly))
        {
            QTextStream stream(&priFile);
            while(!stream.atEnd())
            {
                line = stream.readLine();
                if(line.startsWith(message + ":"))
                {
                    int colonIndex = line.indexOf(":");
                    out = line.mid(colonIndex + 1).trimmed();
                    qDebug() << "getMess 找到字段:" << message << " = " << out;
                    out = removeSymbols2(out,'\"');
                    out = removeSymbols2(out,',');
                    out = removeSymbols2(out,'\"');
                    break;
                }
            }
            priFile.close();
        }
        else
        {
            qDebug() << "getMess 无法打开文件:" << priFile.fileName();
        }

        return out;
    }

    QNetworkAccessManager *qnam;
    QNetworkAccessManager *upLoadreq;

    QProcess p;
    QByteArray line;
    QString node_Name;
    QString netName;//内网名称
    QString nodeIp;
    QString serverIp;
    QString action;
    QString fileName;
    QString dir0;
    QString upContent;
    QString downContent;
    QString Main;
    QDir hdir;
    QStringList dirName1;
    QProgressBar *progressbar;
    QNetworkAccessManager *addReply;
    int progressValue;
    QPlainTextEdit *confTextEdit;
    QGroupBox *progressGroup;
    QVBoxLayout *progressLayout;
    QLabel *progressLabel;
    QPushButton *confirmButton;
    QPushButton *cancelButton;
    QPushButton *okButton;
    QLabel *messageLabel;


public slots:
    void Service_reply(int flag);

};

#endif // DOWNLOAD_H
