#include "logindialog.h"
#include "background.h"
#include "download.h"
#include "envcheck.h"
#include <QWidget>
#include <QApplication>
#include <QDialog>
#include <QMainWindow>
#include <QDesktopWidget>
#include <unistd.h>


QFile *gFileLog = NULL;

char *msgHead[]={
    "Debug   ",
    "Warning ",
    "Critical",
    "Fatal   ",
    "Info    "
};

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");

    if(gFileLog){
        QTextStream tWrite(gFileLog);

        QString msgText="[%1]\n[%6]\n[%2:%3]\n %4 : %5\n";
//        msgText = msgText.arg(msgHead[type]).arg(context.file).arg(context.line).arg(context.function).arg(localMsg.constData()).arg(current_date_time);

        msgText = msgText.arg(msgHead[type]).arg(context.file).arg(context.line).arg(context.function).arg(msg).arg(current_date_time);

        //gFileLog->write(msgText.toLocal8Bit(), msgText.length());
        tWrite << msgText << "\n";
    }else{
        fprintf(stderr, "%s | %s | %s:%u, %s | %s\n", msgHead[type], current_date_time.toLocal8Bit().constData(), context.file, context.line, context.function, localMsg.constData());
    }
}

void logSysInit(QString filePath)
{
    gFileLog = new QFile(filePath);
    if (!gFileLog->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)){
        return;
    }
    //初始化自定义日志处理函数myMessageOutput
    qInstallMessageHandler(myMessageOutput);

}




int main(int argc, char *argv[])
{
//          logSysInit("log.txt");
            QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
            QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

            QApplication a(argc, argv);

            // -------------------------------------------------------
            // 启动前环境检查：tincd.exe 存在性 + TAP 网卡驱动安装状态
            // -------------------------------------------------------
            EnvCheck envCheck(nullptr);
            if (!envCheck.runChecks()) {
                // 致命错误或用户主动退出，终止程序
                return 1;
            }

            background w;
            int currentScreen = a.desktop()->screenNumber(&w);//程序所在的屏幕编号
            QRect rect = a.desktop()->screenGeometry(currentScreen);//程序所在屏幕尺寸
            w.move((rect.width() - w.width()) / 2, (rect.height() - w.height()) / 2);//移动到所在屏幕中间
            w.show();
            return a.exec();
}

