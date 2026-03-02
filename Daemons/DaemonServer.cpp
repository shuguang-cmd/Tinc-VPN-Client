#include "DaemonServer.h"
#include <QDebug>
#include <QProcess>
#include <QApplication>
#include <QCoreApplication>
// #include <iostream>

using namespace std;

QString savepath = "d:/Codes/Java/KenDeJi_RuoYi/tinc_cli_gui/windows/Tinc";
QString update_Api;//更新结果Api
QString alive_Api;//心跳信号Api
QString confPack_Path;

DaemonService::DaemonService(int argc, char **argv)
    : QtService<QCoreApplication>(argc, argv, "Daemons")
{
    setServiceDescription("tinc service daemon");
    setServiceFlags(QtServiceBase::CanBeSuspended);
    qDebug()<<"00";
}

void DaemonService::start()
{
    qDebug() << __FUNCTION__;

    qDebug()<<"start";

    sid = getMess("sid");
    id = getMess("id");
    netName = getMess("net_name");
    old_nodeName = getMess("node_name");
    old_nodeIp = getMess("node_ip");

    serverIp = severIp_conf();
    qDebug()<<serverIp;

    update_Api = QString("http://%1/XVntQFJCjc.php/promin/Api/editadd_info").arg(serverIp);
    qDebug()<<update_Api;

    alive_Api = QString("http://%1/XVntQFJCjc.php/promin/Api/keepalive").arg(serverIp);
    qDebug()<<alive_Api;

    /*QString programPath = QDir::currentPath();
    QFileInfo fileInfo(programPath);
    parentDir = fileInfo.dir().path();  // 获取上一级目录的路径
    confPack_Path = parentDir + "/conf_package";*/

    confPack_Path = "d:/Codes/Java/KenDeJi_RuoYi/tinc_cli_gui/windows/demo_win/demo_setup_win/conf_package";

    qDebug()<<sid<<netName<<serverIp<<old_nodeName<<old_nodeIp;

    servicename = "tinc."+netName;
    qDebug()<<servicename;

    QString str = old_nodeIp;
    //获取网关地址
    QRegularExpression re("(\\d+\\.\\d+\\.\\d+)\\.\\d+");
    gatewayIp = str.replace(re, "\\1.0");
    qDebug()<<gatewayIp;

    heartbeatReq = new QNetworkAccessManager;
    connect(heartbeatReq, &QNetworkAccessManager::finished,this,&DaemonService::HeartReply);

    this->setTime();
}

/* 重新配置 */
bool DaemonService::conf_pack(QString netname,QString nodeip)
{
    action = "update";
    qDebug()<<netname;
    qDebug()<<nodeip;
    qDebug()<<action;

    // 调用conf_package
    QProcess process;
    process.setWorkingDirectory(confPack_Path);
    qDebug()<<confPack_Path;
    process.setProcessChannelMode(QProcess::MergedChannels);  // 将输出与错误合并
    process.start("cmd.exe", QStringList() << "/c" << QString(" start conf_package.exe %1 %2 %3 && exit\r\n").arg(action).arg(netName).arg(nodeIp).toUtf8());
    process.waitForFinished(8000);

    // 执行 tasklist 命令来获取所有进程信息
    QProcess T_process;
    T_process.start("tasklist", QStringList() << "/FO" << "CSV" << "/NH");
    T_process.waitForFinished(3000);
    QString output = T_process.readAllStandardOutput();
    QStringList lines = output.split("\r\n",QString::SkipEmptyParts);

    // 遍历每一行，查找匹配进程名称的行
    foreach(QString line, lines) {
        if(line.contains("conf_package")) {
            // 如果找到匹配的进程名称，提取 PID
            QStringList parts = line.split(",");
            QString pid = parts.at(1).trimmed();
            qDebug() << "Found PID:" << pid;

            //QProcess::execute("taskkill /F /PID " + pid);
        }
    }

    qDebug()<<"conf_pack调用";
    return true;
}

//查询服务状态
int DaemonService::checkServiceStatus()
{
    qDebug()<<"check";

    QProcess p;
    QString command = QString("sc query tinc.%1").arg(netName);
    p.start(command);
    p.waitForFinished(3000);

    QByteArray outputAry = p.readAllStandardOutput();
    QString output = QString::fromUtf8(outputAry);
    qDebug()<<output;

    QStringList lines = output.split("\n");
    QString stateLine;

    // 寻找包含STATE关键字的行
    for (const QString &line : lines) {
        if (line.contains("STATE")) {
            stateLine = line;
            break;
        }
    }

    // 按空格切割STATE行，并获取状态部分
    QStringList parts = stateLine.split(":",QString::SkipEmptyParts);
    QString state = parts.at(1).trimmed(); // 第三个部分为状态
    qDebug() << "STATE:" << state;

    if(state == "4 RUNNING") return 1;

    else return 0;
}

//更新文件内容
bool DaemonService::modifyFileContent(QString filePath, QString targetKey, QString newValue)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file:" << filePath << file.errorString();
        return false;
    }

    QTextStream in(&file);
    QString content;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith(targetKey)) {
            line = line.split("=").at(0) + "=" + newValue;
        }
        content += line + "\n";
    }
    file.close();

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qWarning() << "Failed to open file:" << filePath << file.errorString();
        return false;
    }

    QTextStream out(&file);
    out << content;

    return true;
}

/* 删除需重新配置的文件 */
bool DaemonService::deleteFile(QString path,QString fileName)
{
    QDir dir = path;
    QStringList mainfiles = dir.entryList(QStringList(fileName), QDir::Files);
    foreach (QString file, mainfiles) {
        QFile::remove(dir.filePath(file));
        qDebug() << "File removed: " << file;
    }
    return true;
}

/* 修改private文件 */
bool DaemonService::modifyPrivateFile(QString targetKey, QString newValue)
{
    // QString programPath = QDir::currentPath();
    // QFileInfo fileInfo(programPath);
    // QDir parentDir = fileInfo.dir().path();
    // QString parentpath = parentDir.path();

    //验证可修改字段
    if (targetKey != "node_ip" && targetKey != "node_name") {
        qDebug() << "[ERROR] 只能修改node_ip或node_name字段";
        return false;
    }

    QString filePath = "d:/Codes/Java/KenDeJi_RuoYi/tinc_cli_gui/windows/private.txt";

    //读取整个文本文件
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "[ERROR] 无法打开文件:" << file.errorString();
        return false;
    }

    QString fileContent = file.readAll();
    file.close();

    //定位并修改目标字段
    QString pattern = QString("\"%1\": \"(.*)\"").arg(targetKey);
    QRegularExpression regex(pattern);
    QRegularExpressionMatch match = regex.match(fileContent);

    if (!match.hasMatch()) {
        qDebug() << "[ERROR] 未找到" << targetKey << "字段";
        return false;
    }

    QString oldValue = match.captured(1);
    QString newLine = QString("\"%1\": \"%2\"").arg(targetKey).arg(newValue);
    fileContent.replace(match.capturedStart(), match.capturedLength(), newLine);

    //写回文件
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qDebug() << "[ERROR] 无法写入文件:" << file.errorString();
        return false;
    }

    QTextStream out(&file);
    out << fileContent;
    file.close();

    qDebug() << "[SUCCESS] 成功修改" << targetKey
             << "从" << oldValue << "改为" << newValue;
    return true;
}

/* 设置定时器以及套接字 */
void DaemonService::setTime()
{
    //设置定时器，每隔固定时间发送一次
    timer = new QTimer;
    timer0 = new QTimer;

    QObject::connect(timer, &QTimer::timeout,[=]() {
        sendHeartBeat();
    });
    timer->start(1000);

    QObject::connect(timer0, &QTimer::timeout,[=]() {
        ping();
    });
    timer0->start(10000);
}

/* 发送更新成功信号 */
void DaemonService::successReply(QString details,QString key,QString value)
{
    qDebug()<<"edit success";
    QString type = "edit";
    QString result = "success";

    editReply = new QNetworkAccessManager(this);
    QJsonObject json;
    json.insert("type",type);
    json.insert("result",result);
    json.insert("ids",id);
    json.insert("key",key);
    json.insert("value",value);
    json.insert("details",details);
    QJsonDocument document;
    document.setObject(json);
    QByteArray dataArray = document.toJson(QJsonDocument::Compact);
    /* -------------------------------------- */
    QNetworkRequest editreq;
    editreq.setUrl(QUrl(update_Api));
    editreq.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    editReply -> post(editreq,dataArray);

    qDebug()<<"edit end";
}

/* 发送更新失败信号 */
void DaemonService::failureReply(QString details)
{
    qDebug()<<"edit fail";
    QString type = "edit";
    QString result = "error";
    QString config_info = nullptr;

    editReply = new QNetworkAccessManager;
    QJsonObject json;
    json.insert("type",type);
    json.insert("result",result);
    json.insert("ids",sid);
    json.insert("details",details);
    json.insert("config_info",config_info);
    QJsonDocument document;
    document.setObject(json);
    QByteArray dataArray = document.toJson(QJsonDocument::Compact);
    /* -------------------------------------- */
    QNetworkRequest editreq;
    editreq.setUrl(QUrl(update_Api));
    editreq.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    editReply -> post(editreq,dataArray);
}

/* 发送心跳信号 */
void DaemonService::sendHeartBeat()
{
    qDebug()<<"heart";


    QJsonObject json;
    QString Heart = "Heart";
    json.insert("Heart",Heart);
    json.insert("sid",sid);
    QJsonDocument document;
    document.setObject(json);
    QByteArray dataArray = document.toJson(QJsonDocument::Compact);
    /* -------------------------------------- */
    QNetworkRequest req;
    req.setUrl(QUrl(alive_Api));
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");


    // 改get为post
    heartbeatReq -> post(req,dataArray);
}

/* 更新操作 */
void DaemonService::HeartReply(QNetworkReply* reply)
{
    if(reply->error() == QNetworkReply::NoError){
        QByteArray data = reply->readAll();
        QString response = QString::fromUtf8(data);
        qDebug()<<"keepalive response: " + response;

        //正常
        if(response == "1") return;

        //服务端错误
        else if(response == "-1")
        {
            sendHeartBeat();
        }

        //2,断开连接
        else if(response == "2"){
            qDebug()<<"Intranet disconnection！";
        }

        //更新
        else{
            qDebug()<<"in update";

            if(!response.isEmpty())
            {
                qDebug()<< response;
                int flag = 0;
                QString node_FilePath = (savepath + "/%1/hosts/%2").arg(netName).arg(old_nodeName);
                qDebug()<<node_FilePath;
                QString host_path = (savepath + "/%1").arg(netName);
                qDebug()<<host_path;
                QFile nodeFile(node_FilePath);

                if(nodeFile.open(QIODevice::ReadOnly | QIODevice::Text))
                {
                    qDebug()<<"nodefile";
                    //获取更新内容
                    QStringList update_list = response.split("：",QString::SkipEmptyParts);
                    QString update = update_list[0];
                    QString newValue = update_list[1];
                    qDebug()<< update;
                    qDebug()<<newValue;

                    //更新节点ip
                    if(update == "Subnet"){
                        nodeIp = newValue;
                        qDebug()<<nodeIp;

                        if(modifyFileContent(node_FilePath,update,newValue)){
                            qDebug()<<"file update";
                            if(modifyPrivateFile("node_ip",nodeIp))
                            {
                                qDebug()<<"privatefile update";
                                deleteFile(host_path,"rsa_key.priv");
                                deleteFile(node_FilePath,"main");
                                if(conf_pack("",nodeIp))
                                {
                                    qDebug()<<"reconfigure";
                                    flag = 1;
                                }

                            }
                        }
                        else{
                            qDebug()<<"节点文件更新失败";

                            failureReply("节点文件更新失败");
                        }
                    }

                    //更新节点名称
                    if(update == "Name")
                    {
                        QString conf_filePath = (savepath + "/%1/tinc.conf").arg(netName);
                        nodeName = newValue;
                        qDebug()<<nodeName;

                        if(modifyFileContent(conf_filePath,update,newValue)){
                            qDebug()<<"file update";

                            QProcess p;
                            QString node_path = (savepath + "/%1/hosts").arg(netName);
                            p.setWorkingDirectory(node_path);
                            QString command = QString("rename %1 %2").arg(old_nodeName).arg(nodeName);
                            qDebug()<<command;
                            p.start(command);
                            p.waitForFinished();
                            qDebug()<<nodeFile.fileName();

                            if(modifyPrivateFile("node_name",nodeName))
                            {
                                qDebug()<<"privatefile update";
                                deleteFile(host_path,"rsa_key.priv");
                                deleteFile(node_FilePath,"main");
                                if(conf_pack(nodeName,""))
                                {
                                    qDebug()<<"reconfigure";
                                    flag = 2;
                                }
                            }
                        }
                        else{
                            qDebug()<<"conf文件更新失败";

                            failureReply("conf文件更新失败");
                        }
                    }
                 }
                else
                {
                    qDebug() << "打开失败";
                }
                    //重启服务进行测试
                    QProcess process(0);
                    QString sCmd = QString("net start %1").arg(servicename);
                    qDebug()<< sCmd;
                    process.setWorkingDirectory(savepath);
                    process.start(sCmd);
                    process.waitForFinished();
                    if(checkServiceStatus() == 0)
                    {
                        failureReply("服务开启失败，更新失败");
                    }
                    else
                    {
                        qDebug()<<flag;
                        if(flag == 1)
                        {
                            qDebug()<<"ip";
                            QString details = QString("节点%1修改节点ip成功。old:%2,new:%3 已完成重新配置").arg(sid).arg(old_nodeIp).arg(nodeIp);
                            successReply(details,"node_ip",nodeIp);
                            old_nodeIp = nodeIp;
                        }
                        else if(flag == 2)
                        {
                            qDebug()<<"name";
                            QString details = QString("节点%1修改节点名称成功。old:%2,new:%3 已完成重新配置").arg(sid).arg(old_nodeName).arg(nodeName);
                            successReply(details,"node_name",nodeName);
                            old_nodeName = nodeName;
                        }
                    }
                }
            }
        }
    else{
        qDebug()<<"failed:"<<reply->errorString();
    }
}

/* 异常检查 */
void DaemonService::ping(){

    //ping网关
    QProcess process(0);
    QString pCmd = QString("ping %1").arg(gatewayIp);
    qDebug()<< pCmd;
    process.setWorkingDirectory(savepath);
    process.start(pCmd);
    process.waitForFinished();
    QString outPut = process.readAll();

    if(outPut.contains("不可达") || outPut.contains("Unreachable"))
    {
        qDebug()<<"ping failure";

        timer->stop();

        //ping不通则查询异常
        QString cfilePath = savepath + "/" + netName + "/tinc.conf";
        qDebug()<<cfilePath;
        QFile confFile(cfilePath);
        //查询conf文件是否存在
        if(!confFile.exists()){
            qDebug()<<"The File 'tinc.conf' Is Missing!";
        }

        //查询hosts文件夹是否存在
        QString folderPath = savepath + "/" + netName + "/hosts";
        QDir hostsFolder(folderPath);
        if(hostsFolder.exists()){
            //查询节点文件是否存在
            QString nfilePath = (savepath + "/" + netName + "/hosts/%1").arg(nodeName);
            qDebug()<<nfilePath;
            QFile nodeFile(nfilePath);
            if(!nodeFile.exists()){
                qDebug()<<"The nodeFile Is Missing!";
            }

            //查询main文件是否存在
            QString mfilePath = savepath + "/" + netName + "/hosts/main";

            qDebug()<<mfilePath;
            QFile mainFile(mfilePath);
            if(!mainFile.exists()){
                qDebug()<<"The File 'main' Is Missing!";
            }
        }
        else{
            qDebug()<<"The Folder 'hosts' Is Missing!";
        }

        qDebug()<<"here2";

        //查询服务是否有问题
        if(!checkServiceStatus())
        {
            qDebug()<<"Service connection error!";

        }
    }
    else
    {
        if(!timer->isActive())
        {
            timer->start();
        }
        qDebug()<<"ping success";
    }
}

void DaemonService::stop()
{
    qDebug() << __FUNCTION__;
}

