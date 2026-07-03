#include "conf_package.h"
#include <QDir>
#include <QProcess>
#include <QDebug>
#include <QNetworkRequest>
#include <QFile>
#include <QCoreApplication>
#include <QThread>

// 全局配置路径：Tinc VPN服务安装目录
QString my_savePath;
// 网络名称：从命令行参数传入，不再硬编码
QString my_netName = "";
// 服务器IP：从命令行参数传入，用于上传公钥
QString my_serverIp = "";

/**
 * @brief conf_package构造函数
 * @param tem_SId 节点ID（用户名/客户端标识符）
 * @param tem_token 认证令牌，用于验证客户端身份
 * @param tem_netName 网络名称，指定Tinc VPN网络
 * @param tem_nodeIp 节点IP地址，分配给客户端的虚拟IP
 * @param tem_action 操作类型（如"add"表示添加节点）
 * @param tem_key 配置键（预留参数，当前未使用）
 * @param tem_value 配置值（预留参数，当前未使用）
 * @param parent 父窗口对象
 * 
 * 功能说明：
 * 1. 初始化网络访问管理器，用于HTTP请求
 * 2. 连接网络请求完成信号到槽函数
 * 3. 自动启动密钥生成流程
 */
conf_package::conf_package(const QString& tem_SId,const QString& tem_token,const QString& tem_netName,const QString& tem_nodeIp,const QString& tem_action,const QString& tem_key,const QString& tem_value,const QString& tem_serverIp,QWidget *parent)
    : QMainWindow(parent),SId(tem_SId),token(tem_token),netName(tem_netName),nodeIp(tem_nodeIp),action(tem_action),key(tem_key),value(tem_value),serverIp(tem_serverIp)
{
    // 动态自适应定位根目录并设置 Tinc 路径
    QDir searchDir(QCoreApplication::applicationDirPath());
    QString parentpath;
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
        QDir fallback(QCoreApplication::applicationDirPath());
        fallback.cdUp();
        parentpath = fallback.absolutePath();
    }
    my_savePath = QDir(parentpath).absoluteFilePath("Tinc");
    qDebug() << "conf_package Tinc 路径:" << my_savePath;

    // 初始化全局网络名称为传入的参数值
    my_netName = tem_netName;
    // 初始化全局服务器IP为传入的参数值
    my_serverIp = tem_serverIp;
    
    // 创建网络访问管理器，用于发送HTTP请求
    manager = new QNetworkAccessManager(this);
    // 连接信号：当网络请求完成时，调用onUploadFinished槽函数
    connect(manager, &QNetworkAccessManager::finished, this, &conf_package::onUploadFinished);
    qDebug()<<"Starting key generation";
    // 自动开始密钥生成流程
    generate_key();
}

/**
 * @brief 生成RSA密钥对
 * 
 * 功能说明：
 * 1. 检查并创建必要的目录结构（网络目录和hosts子目录）
 * 2. 调用tincd.exe生成RSA密钥对
 * 3. 验证密钥生成是否成功
 * 4. 成功后自动触发公钥上传流程
 * 
 * 技术细节：
 * - 使用QProcess调用tincd.exe命令行工具
 * - 参数 -n 指定网络名称
 * - 参数 -K 表示生成密钥对
 * - 工作目录设置为网络目录，确保密钥文件生成在正确位置
 * 
 * 生成的文件：
 * - rsa_key.priv：私钥文件，保存在网络目录下
 * - hosts/<节点名>：公钥文件，保存在hosts子目录下
 */
void conf_package::generate_key(){
    qDebug() << "Generating keys...";
    
    // 构建tincd.exe的完整路径
    QString tincdPath = QDir(my_savePath).absoluteFilePath("tincd.exe");
    qDebug() << "准备启动的工具绝对路径:" << tincdPath;
    qDebug() << "该文件真的存在吗？:" << QFile::exists(tincdPath);
    
    // 构建工作目录：Tinc根目录/网络名称
    QString workDir = my_savePath + "\\" + my_netName;
    qDebug()<< "Working directory:" << workDir;
    
    // 检查并创建网络目录
    QDir dir;
    if (!dir.exists(workDir)) {
        dir.mkpath(workDir);
        qDebug() << "Created directory:" << workDir;
    }
    
    // 检查并创建hosts子目录
    QString hostsDir = workDir + "\\hosts";
    if (!dir.exists(hostsDir)) {
        dir.mkpath(hostsDir);
        qDebug() << "Created hosts directory:" << hostsDir;
    }
    
    // 1. 斩草除根：删除旧密钥文件
    QString oldPrivKey = workDir + "\\rsa_key.priv";
    QString oldPubKey = hostsDir + "\\" + SId;
    
    if (QFile::exists(oldPrivKey)) {
        QFile::remove(oldPrivKey);
        qDebug() << "Removed old private key:" << oldPrivKey;
    }
    if (QFile::exists(oldPubKey)) {
        QFile::remove(oldPubKey);
        qDebug() << "Removed old public key:" << oldPubKey;
    }
    
    // ==========================================
    // 2. 启动进程生成密钥
    // ==========================================
    QProcess p;
    p.setWorkingDirectory(workDir); // 设置工作目录极为重要
    
    qDebug() << "开始启动 tincd -K...";
    qDebug() << "准备启动的工具绝对路径:" << tincdPath;
    qDebug() << "该文件真的存在吗？:" << QFile::exists(tincdPath);
    p.start(tincdPath, QStringList() << "-n" << my_netName << "-K");
    
    if (!p.waitForStarted(3000)) {
        qDebug() << "致命错误：无法启动 tincd.exe，请检查路径是否正确！";
        return;
    }
    
    // 稍微等一下，等 tincd 吐出提示信息
    QThread::msleep(500); 

    // 【致命修补 1 & 2】：灌入两次回车，并立即关闭写入通道！
    qDebug() << "向进程注入双回车...";
    p.write("\n\n"); 
    p.closeWriteChannel(); // 告诉 tincd 输入已结束，别等了！
    
    // 等待进程完成
    if (!p.waitForFinished(15000)) {
        qDebug() << "tincd process timeout! (可能依然卡在交互提示上)";
        p.kill();
        return;
    }
    
    int exitCode = p.exitCode();
    qDebug() << "tincd exit code:" << exitCode;
    qDebug() << "tincd output:\n" << QString::fromLocal8Bit(p.readAllStandardOutput());
    QString errorOutput = QString::fromLocal8Bit(p.readAllStandardError());
    if (!errorOutput.isEmpty()) {
        qDebug() << "tincd error:\n" << errorOutput;
    }
    
    if (exitCode != 0) {
        qDebug() << "tincd failed with exit code:" << exitCode;
        return;
    }
    
    qDebug() << "纯 RSA 密钥生成成功，准备注入 Subnet...";

   // ==========================================
    // 3. 【致命修补 3】：强行向公钥注入 Subnet
    // ==========================================
    
    // 打印出来看看，到底传进来的 nodeIp 是个啥！
    qDebug() << "准备注入公钥，当前类中保存的 nodeIp 是：" << nodeIp;

    QFile hostFile(oldPubKey); // 就是刚才生成的 hosts 下的文件
    if (hostFile.open(QIODevice::ReadWrite | QIODevice::Text)) {
        QByteArray keyContent = hostFile.readAll(); 
        
        hostFile.resize(0); // 清空
        
        QTextStream out(&hostFile);
        
        // 如果发现传进来的还是 127.0.0.1 或者空的，那就在这里写死做个终极测试！
        if (nodeIp.isEmpty() || nodeIp == "127.0.0.1") {
            qDebug() << "警告！传进来的 IP 依然不对，强制使用硬编码测试 IP！";
            out << "Subnet = 12.12.12.12/32\n\n"; 
        } else {
            out << "Subnet = " << nodeIp << "/32\n\n"; 
        }
        
        out << keyContent; 
        
        hostFile.close();
        qDebug() << "成功注入 Subnet。";
    } else {
        qDebug() << "警告：无法打开公钥文件进行 Subnet 注入！路径：" << oldPubKey;
    }
    
    // 4. 上传公钥到服务器
    // ==========================================
    uploadPublicKey();
}

/**
 * @brief 上传公钥到服务器
 * 
 * 功能说明：
 * 1. 读取生成的公钥文件内容
 * 2. 构造JSON格式的HTTP请求
 * 3. 发送POST请求到服务器接口
 * 4. 等待服务器响应
 * 
 * 请求参数：
 * - sid：节点ID
 * - token：认证令牌
 * - netName：网络名称
 * - nodeIp：节点IP地址
 * - action：操作类型（"add"）
 * - content：公钥内容
 * 
 * 技术细节：
 * - 使用QNetworkAccessManager发送HTTP请求
 * - Content-Type设置为application/json
 * - 请求体为JSON格式，包含公钥内容
 * - 服务器接口：/api/tinc/client/key/upload
 */
void conf_package::uploadPublicKey(){
    qDebug() << "Uploading public key...";
    
    // 构建公钥文件路径：网络目录/hosts/节点名（使用Windows原生路径分隔符）
    QString publicKeyPath = my_savePath + "\\" + my_netName + "\\hosts\\" + SId;
    QFile publicKeyFile(publicKeyPath);
    
    // 尝试打开公钥文件
    if (!publicKeyFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open public key file:" << publicKeyPath;
        return;
    }
    
    // 读取公钥内容
    QString publicKeyContent = QString::fromUtf8(publicKeyFile.readAll());
    publicKeyFile.close();
    
    qDebug() << "Public key content:" << publicKeyContent;
    
    // 新版RESTful API地址
    // POST /api/tinc/client/key/upload
    // 响应：纯文本 (text/plain)，即 server_master 主机配置文件内容
    // 根据 IP 类型动态判断协议（本地/内网用 http，外网用 https）
    QString protocol = (my_serverIp.startsWith("127.0.0.1") || my_serverIp.startsWith("localhost") || my_serverIp.startsWith("192.168.") || my_serverIp.startsWith("10.")) ? "http" : "https";
    QString url = protocol + "://" + my_serverIp + "/api/tinc/client/key/upload";
    qDebug() << "Upload URL:" << url;
    
    // 新版请求体：只需 sid、content、action 三个字段
    // token、netName、nodeIp 已从新接口移除
    QJsonObject json;
    json.insert("sid", SId);                   // 节点名
    json.insert("content", publicKeyContent);  // 公钥 PEM 内容（含 Subnet 头）
    json.insert("action", "exchangeFile");      // 固定操作类型
    
    // 将JSON对象转换为字节数组
    QJsonDocument document;
    document.setObject(json);
    QByteArray dataArray = document.toJson(QJsonDocument::Compact);
    
    qDebug() << "Request JSON:" << dataArray;
    
    // 创建HTTP请求
    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    // 发送POST请求
    QNetworkReply *reply = manager->post(request, dataArray);
    
    qDebug() << "Upload request sent";
}


/**
 * @brief 公钥上传完成后的回调函数
 * @param reply 网络响应对象
 * 
 * 功能说明：
 * 1. 检查上传是否成功
 * 2. 读取服务器响应
 * 3. 记录日志信息
 * 4. 清理网络响应对象
 * 5. 退出应用程序
 * 
 * 技术细节：
 * - 通过检查reply->error()判断请求是否成功
 * - NoError表示请求成功
 * - 读取响应内容并记录到日志
 * - 使用deleteLater()安全删除响应对象
 * - 最后调用QCoreApplication::quit()退出程序
 */
void conf_package::onUploadFinished(QNetworkReply* reply){
    qDebug() << "Upload finished";
    
    // 检查是否有错误
    if (reply->error() == QNetworkReply::NoError) {
        // 新接口返回的是纯文本（text/plain），即 server_master 主机配置文件内容
        QByteArray response = reply->readAll();
        QString serverMasterContent = QString::fromUtf8(response);
        qDebug() << "Server response (server_master content):" << serverMasterContent;
        
        // 将服务器返回的主机配置保存为 hosts/server_master 文件
        QString serverMasterPath = my_savePath + "\\" + my_netName + "\\hosts\\server_master";
        QFile serverMasterFile(serverMasterPath);
        if (serverMasterFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream out(&serverMasterFile);
            out << serverMasterContent;
            serverMasterFile.close();
            qDebug() << "server_master file saved to:" << serverMasterPath;
        } else {
            qDebug() << "Failed to write server_master file:" << serverMasterFile.errorString();
        }
        
        qDebug() << "Public key uploaded successfully, server_master saved";
    } else {
        // 记录错误信息
        qDebug() << "Upload error:" << reply->errorString();
    }
    
    // 安全删除响应对象
    reply->deleteLater();
    
    qDebug() << "Exiting conf_package";
    // 退出应用程序
    QCoreApplication::quit();
}


/**
 * @brief 析构函数
 * 
 * 功能说明：
 * - 清理资源，释放内存
 * - 当前为空实现，Qt会自动管理子对象
 */
conf_package::~conf_package()
{

}
