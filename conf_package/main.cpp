#include "conf_package.h"
#include <QApplication>
#include <QMutex>
#include <QMutexLocker>
#include <QFile>
#include <QTextStream>
#include <QDebug>

QString tem_netName;
QString tem_nodeIp;
QString tem_action;
QString tem_key;
QString tem_value;
QString tem_SId;
QString tem_token;

//重定向qdebug输出到文件
//重定向qdebug输出到文件
//重定向qdebug输出到文件
void myMessageHandle(QtMsgType , const QMessageLogContext& , const QString& msg)
{
    static QMutex mut; //多线程打印时需要加锁
    QMutexLocker locker(&mut);
    
    // ✅ 动态获取 exe 所在目录，将日志写在当前目录下
    QString logPath = QCoreApplication::applicationDirPath() + "/conf_log.txt";
    QFile file(logPath);
    
    if(file.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        QTextStream stream(&file);
        stream << msg << "\n"; // Qt 5.15+ 推荐用 "\n" 替代 endl
        stream.flush();
        file.close();
    }
    else
    {
        // 如果无法打开日志文件，尝试输出到标准错误
        QTextStream stderrStream(stderr);
        stderrStream << "无法打开日志文件: " << logPath << "\n";
        stderrStream << "日志内容: " << msg << "\n";
    }
}

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
    QString parentpath = "d:/Codes/Java/KenDeJi_RuoYi/tinc_cli_gui/windows";
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
            if(line.startsWith(message + ":"))
            {
                int colonIndex = line.indexOf(":");
                out = line.mid(colonIndex + 1);
                qDebug()<<out;
                out = removeSymbols2(out,'\"');
                out = removeSymbols2(out,',');
                out = removeSymbols2(out,'\"');
                break;
            }
        }
        priFile.close();
    }

    return out;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    //设置重定向操作的函数
    qInstallMessageHandler(myMessageHandle);

    // 放宽限制：只要参数有 6 个（程序名+5个核心参数）就允许运行
    if (argc < 6) {
        qDebug() << "参数不足！当前参数个数：" << argc;
        return -1;
    }

    // 安全接收参数，防止数组越界崩溃
    QString sid = QString::fromUtf8(argv[1]);
    QString token = QString::fromUtf8(argv[2]);
    QString netName = QString::fromUtf8(argv[3]);
    QString nodeIp = QString::fromUtf8(argv[4]);
    QString action = QString::fromUtf8(argv[5]);
    
    // 安全判断，避免崩溃
    QString key = "";
    QString value = "";
    if (argc > 6) {
        key = QString::fromUtf8(argv[6]);
        if (key == "none") key = ""; // 把占位符还原为空
    }
    if (argc > 7) {
        value = QString::fromUtf8(argv[7]);
        if (value == "none") value = ""; // 把占位符还原为空
    }
    // 从private.txt读取服务器IP

    
    QString serverIp = getMess("server_ip");

    qDebug() << "接收到的参数:";
    qDebug() << "  sid:" << sid;
    qDebug() << "  token:" << token;
    qDebug() << "  netName:" << netName;
    qDebug() << "  nodeIp:" << nodeIp;
    qDebug() << "  action:" << action;
    qDebug() << "  key:" << key;
    qDebug() << "  value:" << value;
    qDebug() << "  serverIp:" << serverIp;

    conf_package w(sid, token, netName, nodeIp, action, key, value, serverIp);

    //核心修复：踩下油门，触发核心逻辑！
    w.generate_key();

    return a.exec();
}
