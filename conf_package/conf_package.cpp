#include "conf_package.h"
#include <QDir>
#include <QProcess>
#include <QDebug>
#include <QNetworkRequest>
#include <QFile>
#include <QCoreApplication>
#include <QThread>

// 全局配置路径：Tinc VPN服务安装目录
QString my_savePath = "d:/Codes/Java/KenDeJi_RuoYi/tinc_cli_gui/windows/Tinc";
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
    
    // 构建tincd.exe的完整路径（使用Windows原生路径分隔符）
    QString tincdPath = my_savePath + "\\tincd.exe";
    qDebug()<< "tincd path:" << tincdPath;
    
    // 构建工作目录：Tinc根目录/网络名称（使用Windows原生路径分隔符）
    QString workDir = my_savePath + "\\" + my_netName;
    qDebug()<< "Working directory:" << workDir;
    
    // 检查并创建网络目录
    QDir dir;
    if (!dir.exists(workDir)) {
        dir.mkpath(workDir);
        qDebug() << "Created directory:" << workDir;
    }
    
    // 检查并创建hosts子目录（用于存放公钥文件）
    QString hostsDir = workDir + "\\hosts";
    if (!dir.exists(hostsDir)) {
        dir.mkpath(hostsDir);
        qDebug() << "Created hosts directory:" << hostsDir;
    }
    
    // 创建进程对象并设置工作目录
    QProcess p;
    p.setWorkingDirectory(workDir);
    
    // 启动tincd.exe生成密钥：-n 指定网络名，-K 表示生成密钥
    p.start(tincdPath, QStringList() << "-n" << my_netName << "-K");
    
    // 检查进程是否成功启动
    if (!p.waitForStarted()) {
        qDebug() << "Failed to start tincd process";
        return;
    }
    
    // 等待tincd.exe提示输入（等待2秒让tincd.exe启动）
    QThread::msleep(2000);
    
    // 提供回车键来确认默认的保存位置
    p.write("\r\n");
    
    // 等待进程完成，最多等待30秒
    if (!p.waitForFinished(30000)) {
        qDebug() << "tincd process timeout";
        p.kill();
        return;
    }
    
    // 获取进程退出码和输出信息
    int exitCode = p.exitCode();
    QString output = QString::fromLocal8Bit(p.readAllStandardOutput());
    QString error = QString::fromLocal8Bit(p.readAllStandardError());
    
    qDebug() << "tincd exit code:" << exitCode;
    qDebug() << "tincd output:" << output;
    if (!error.isEmpty()) {
        qDebug() << "tincd error:" << error;
    }
    
    // 检查密钥生成是否成功（退出码为0表示成功）
    if (exitCode != 0) {
        qDebug() << "tincd failed with exit code:" << exitCode;
        return;
    }
    
    qDebug() << "Key generation completed successfully";
    
    // 密钥生成成功，开始上传公钥
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
 * - 服务器接口：/XVntQFJCjc.php/coreplugs/Clientinterface/exchangeFile
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
    
    // 构建服务器URL（使用从private.txt读取的动态服务器地址）
    QString url = "http://" + my_serverIp + "/XVntQFJCjc.php/coreplugs/Clientinterface/exchangeFile";
    qDebug() << "Upload URL:" << url;
    
    // 构造JSON请求体
    QJsonObject json;
    json.insert("sid", SId);           // 节点ID
    json.insert("token", token);       // 认证令牌
    json.insert("netName", netName);   // 网络名称
    json.insert("nodeIp", nodeIp);     // 节点IP
    json.insert("action", "add");      // 操作类型：添加节点
    json.insert("content", publicKeyContent); // 公钥内容
    
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
        // 读取服务器响应
        QByteArray response = reply->readAll();
        qDebug() << "Server response:" << response;
        qDebug() << "Public key uploaded successfully";
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
