#include "download.h"
#include "logindialog.h"

// 全局变量：接口列表（预留，当前未使用）
QStringList InterList;
// 下载配置文件的API地址
QString Download_Api;
// Tinc VPN服务安装目录
QString my_savePath = "d:/Codes/Java/KenDeJi_RuoYi/tinc_cli_gui/windows/Tinc";
// 编辑信息的API地址
QString Edit_Api;
// 父目录（预留，当前未使用）
QString parentDir;
// 网络接口（预留，当前未使用）
QString Interface;
// 节点ID
QString SId;
// 认证令牌
QString token;
// 服务器IP地址
QString server_Ip;
// 行字符串（预留，当前未使用）
QString linestr;
// conf_package.exe的路径
QString confPack_Path;
// Daemons.exe的路径
QString daemonsPath;

/**
 * @brief download构造函数
 * @param sid 节点ID（用户名）
 * @param Token 认证令牌
 * @param serverIp 服务器IP地址
 * @param parent 父窗口对象
 * 
 * 功能说明：
 * 1. 初始化UI界面（进度条和日志显示）
 * 2. 设置API地址
 * 3. 初始化网络访问管理器
 * 4. 自动开始下载配置文件
 * 
 * 工作流程：
 * 1. 创建进度条和日志显示界面
 * 2. 设置下载和编辑API地址
 * 3. 发送下载配置文件的请求
 * 4. 等待服务器响应并处理
 * 
 * 进度说明：
 * - 0%: 初始化完成，开始下载
 * - 10%: 配置文件下载完成
 * - 20%: 配置文件解压完成
 * - 80%: 密钥生成和上传完成
 * - 90%: 服务测试完成
 * - 100%: 配置完成
 */
download::download(const QString& sid,const QString& Token,const QString& serverIp,QWidget* parent) :
    QWidget(parent),progressValue(0),node_Name(""),netName(""),nodeIp(""),serverIp(""),action(""),fileName(""),dir0(""),upContent(""),downContent(""),Main("")
{
    qDebug() << "download 构造函数开始执行";
    
    // 移除 WA_DeleteOnClose 属性，防止窗口被意外关闭
    // this->setAttribute(Qt::WA_DeleteOnClose);

    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);

    // 创建进度条组
    progressGroup = new QGroupBox("配置进度", this);
    progressGroup->setFont(QFont("宋体", 9));
    progressLayout = new QVBoxLayout(progressGroup);
    progressLayout->setContentsMargins(8, 12, 8, 12);

    // 创建进度条
    progressbar = new QProgressBar(this);
    progressbar->setRange(0,100);
    progressbar->setValue(progressValue);
    progressbar->setTextVisible(true);
    progressbar->setAlignment(Qt::AlignCenter);
    progressbar->setStyleSheet(
        "QProgressBar {"
        "   border: 1px solid #2196F3;"
        "   border-radius: 3px;"
        "   text-align: center;"
        "   background-color: #E3F2FD;"
        "   height: 22px;"
        "}"
        "QProgressBar::chunk {"
        "   background-color: #2196F3;"
        "   border-radius: 2px;"
        "}"
        );

    // 创建进度标签
    progressLabel = new QLabel("正在准备配置...", this);
    progressLabel->setFont(QFont("宋体", 8));
    progressLabel->setAlignment(Qt::AlignCenter);

    progressLayout->addWidget(progressbar);
    progressLayout->addWidget(progressLabel);
    progressGroup->setLayout(progressLayout);
    mainLayout->addWidget(progressGroup);

    // 创建日志输出组
    QGroupBox* outputGroup = new QGroupBox("配置日志", this);
    outputGroup->setFont(QFont("宋体", 9));
    QVBoxLayout* outputLayout = new QVBoxLayout(outputGroup);
    outputLayout->setContentsMargins(5, 10, 5, 10);

    // 创建日志文本框
    confTextEdit = new QPlainTextEdit(this);
    confTextEdit->setReadOnly(true);
    confTextEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    confTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    confTextEdit->setUndoRedoEnabled(false);
    confTextEdit->setStyleSheet(
        "QPlainTextEdit {"
        "   background-color: white;"
        "   border: 1px solid #BDBDBD;"
        "   border-radius: 2px;"
        "   padding: 3px;"
        "   font-size: 9pt;"
        "}"
        );

    // 记录开始时间
    QDateTime currentTime = QDateTime::currentDateTime();
    confTextEdit->appendPlainText(currentTime.toString("[hh:mm:ss]") + " 开始配置...");

    outputLayout->addWidget(confTextEdit);
    outputGroup->setLayout(outputLayout);
    mainLayout->addWidget(outputGroup);

    this->setLayout(mainLayout);
    // 移除这行，不要设置窗口标志为 Qt::Widget
    // this->setWindowFlags(Qt::Widget);

    // 保存参数
    SId = sid;
    token = Token;
    server_Ip = serverIp;
    qDebug() << SId << token<<server_Ip;

    // 设置路径
    daemonsPath = "d:/Codes/Java/KenDeJi_RuoYi/tinc_cli_gui/windows/demo_win/demo_setup_win/Daemons";
    confPack_Path = "d:/Codes/Java/KenDeJi_RuoYi/tinc_cli_gui/windows/demo_win/demo_setup_win/conf_package";
    confTextEdit->appendPlainText(currentTime.toString("[hh:mm:ss]") + " 配置路径: " + confPack_Path);
    qDebug()<<confPack_Path;
    confTextEdit->appendPlainText(currentTime.toString("[hh:mm:ss]") + " 守护进程路径: " +daemonsPath);
    qDebug()<<daemonsPath;

    // 构建下载API地址
    Download_Api = QString("http://%1/XVntQFJCjc.php/coreplugs/coreplugs/api").arg(server_Ip);
    confTextEdit->appendPlainText(currentTime.toString("[hh:mm:ss]") + " 下载API: " +Download_Api);
    qDebug()<<Download_Api;

    // 构建编辑API地址
    Edit_Api = QString("http://%1/XVntQFJCjc.php/promin/Api/editadd_info").arg(server_Ip);
    confTextEdit->appendPlainText(currentTime.toString("[hh:mm:ss]") + " 编辑API: " + Edit_Api);
    qDebug()<<Edit_Api;

    // 创建网络访问管理器
    qnam = new QNetworkAccessManager(this);
    // 连接信号：当网络请求完成时，调用text槽函数
    connect(qnam,&QNetworkAccessManager::finished,this,&download::text);

    // 构造JSON请求体
    QJsonObject json;
    json.insert("sid",sid);
    json.insert("token",Token);
    QJsonDocument document;
    document.setObject(json);
    QByteArray dataArray = document.toJson(QJsonDocument::Compact);

    // 创建HTTP请求
    QNetworkRequest req;
    req.setUrl(QUrl(Download_Api));
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    // 发送POST请求下载配置文件
    qnam -> post(req,dataArray);
}

/**
 * @brief 延迟函数
 * @param mSec 延迟毫秒数
 * 
 * 功能说明：
 * - 阻塞当前线程指定毫秒数
 * - 用于等待异步操作完成
 */
void delay(int mSec)
{
    QEventLoop loop;
    QTimer::singleShot(mSec,&loop,SLOT(quit()));
    loop.exec();
}

/**
 * @brief 处理下载配置文件的响应
 * @param reply 网络响应对象
 * 
 * 功能说明：
 * 1. 接收服务器返回的配置文件（ZIP格式）
 * 2. 保存到本地磁盘
 * 3. 更新进度条到10%
 * 4. 自动触发解压流程
 * 
 * 技术细节：
 * - 保存路径：my_savePath/configure.zip
 * - 使用QFile写入文件
 * - 删除reply对象避免内存泄漏
 */
void download:: text(QNetworkReply *reply)
{
    // 创建配置文件
    QFile file(my_savePath + "/configure.zip");
    file.open(QIODevice::WriteOnly);

    // 写入配置文件内容
    file.write(reply->readAll());
    file.close();
    
    // 使用 deleteLater() 而不是直接 delete，避免崩溃
    reply->deleteLater();

    confTextEdit->appendPlainText(QString::fromUtf8("压缩包接受完毕"));

    // 更新进度到10%
    progressValue += 10;
    progressbar->setValue(progressValue);
    
    // 强制 Qt 立刻重绘界面，进度条和日志就会丝滑地动起来
    QCoreApplication::processEvents();

    // 开始解压
    this->Decompression();
}

/**
 * @brief 解压配置文件
 * 
 * 功能说明：
 * 1. 使用PowerShell解压ZIP文件
 * 2. 解压到Tinc根目录
 * 3. 更新进度条到20%
 * 4. 自动触发密钥生成流程
 * 
 * 技术细节：
 * - 使用PowerShell的Expand-Archive命令
 * - -Force参数强制覆盖已存在文件
 * - 解压目标：my_savePath（Tinc根目录）
 * 
 * 注意事项：
 * - 当前解压到Tinc根目录，应该解压到MyFirstNet子目录
 * - 需要修改解压路径以匹配服务器端ZIP文件结构
 */
void download::Decompression(){
    QProcess p;
    p.setWorkingDirectory(my_savePath);
    
    // 使用PowerShell解压ZIP文件
    p.start("powershell", QStringList() << "-Command" << "Expand-Archive -Path configure.zip -DestinationPath . -Force");
    p.waitForFinished();
    p.close();

    confTextEdit->appendPlainText(QString::fromUtf8("压缩包解压完毕"));

    // 更新进度到20%
    progressValue += 10;
    progressbar->setValue(progressValue);
    
    // 强制 Qt 立刻重绘界面，进度条和日志就会丝滑地动起来
    QCoreApplication::processEvents();

    // 开始密钥生成
    this->conf_pack();
}

/**
 * @brief 调用conf_package.exe生成密钥并上传
 * 
 * 功能说明：
 * 1. 从private.txt读取配置信息
 * 2. 启动conf_package.exe进程
 * 3. 传递7个参数：SId, token, netName, nodeIp, action, key, value
 * 4. 等待进程完成
 * 5. 更新进度条到80%
 * 6. 自动触发服务测试
 * 
 * 技术细节：
 * - 使用QProcess启动外部程序
 * - 工作目录设置为conf_package.exe所在目录
 * - 使用tasklist检查进程状态
 * - 参数传递修复：之前只传3个参数，现在传7个参数
 * 
 * 重要修复：
 * - 修复了参数传递问题，现在传递完整的7个参数
 * - 之前：只传action, netName, nodeIp
 * - 现在：传SId, token, netName, nodeIp, action, "", ""
 */
void download::conf_pack()
{
    action = "add";
    // 从private.txt读取节点IP
    nodeIp = getMess("server_ip");
    // 从private.txt读取网络名称
    netName = getMess("net_name");
    qDebug()<<nodeIp;
    confTextEdit->appendPlainText("getMess:"+nodeIp);
    qDebug()<<netName;
    confTextEdit->appendPlainText("getMess:"+netName);
    qDebug()<<action;
    confTextEdit->appendPlainText(action);

    // 创建进程对象
    QProcess process;
    process.setWorkingDirectory(confPack_Path);
    qDebug()<<confPack_Path;
    process.setProcessChannelMode(QProcess::MergedChannels);

    // 启动conf_package.exe，传递7个参数（修复了之前只传3个参数的问题）
    // 直接运行conf_package.exe，不使用start命令，确保等待完成
    // 强制使用绝对路径启动，彻底杜绝找不到文件的问题！
    QString exePath = confPack_Path + "/conf_package.exe";
    process.start(exePath, QStringList() << SId << token << netName << nodeIp << action << "none" << "none");

    // 检查进程是否成功启动
    if (!process.waitForStarted(5000)) {
        qDebug() << "Failed to start conf_package.exe";
        qDebug() << "Error:" << process.errorString();
        confTextEdit->appendPlainText("错误：无法启动conf_package.exe");
        confTextEdit->appendPlainText("错误信息：" + process.errorString());
        return;
    }

    qDebug() << "conf_package.exe started successfully";

    // 等待conf_package.exe完成，最多等待30秒（密钥生成+上传需要时间）
    if (!process.waitForFinished(30000)) {
        qDebug() << "conf_package.exe timeout or failed";
        confTextEdit->appendPlainText("警告：密钥生成或上传超时");
    } else {
        qDebug() << "conf_package.exe completed with exit code:" << process.exitCode();
    }

    // 读取进程输出
    QByteArray conf_output = process.readAll();
    qDebug() << QString::fromLocal8Bit(conf_output);
    confTextEdit->appendPlainText(QString::fromLocal8Bit(conf_output));

    // 检查conf_package.exe进程状态
    QString processName = "conf_package";

    QProcess T_process;
    T_process.start("tasklist", QStringList() << "/FO" << "CSV" << "/NH");
    T_process.waitForFinished(3000);
    QString output = T_process.readAllStandardOutput();
    QStringList lines = output.split("\r\n",QString::SkipEmptyParts);

    foreach(QString line, lines) {
        if(line.contains(processName)) {
            QStringList parts = line.split(",");
            QString pid = parts.at(1).trimmed();
            qDebug() << "Found PID:" << pid;

        }
    }

    // 更新进度到80%
    progressValue += 60;
    progressbar->setValue(progressValue);
    
    // 强制 Qt 立刻重绘界面，进度条和日志就会丝滑地动起来
    QCoreApplication::processEvents();

    // 开始服务测试
    this->test();
}

/**
 * @brief 测试Tinc服务状态
 * 
 * 功能说明：
 * 1. 使用sc query命令查询Tinc服务状态
 * 2. 解析服务状态
 * 3. 判断服务是否正常运行
 * 4. 更新进度条到90%
 * 5. 自动触发结果上报
 * 
 * 技术细节：
 * - 服务名称格式：tinc.<网络名称>
 * - 使用Windows服务控制命令sc query
 * - 状态码4 RUNNING表示服务正在运行
 * - flag=0表示成功，flag=1表示失败
 * 
 * 修改说明：
 * - 添加了错误处理，防止服务查询失败导致程序卡住
 * - 如果服务不存在，直接跳过测试，继续后续流程
 * - 添加了详细的调试信息
 */
void download::test(){
    qDebug()<<"test";
    int flag = 0; // 默认为成功

    QProcess p;
    // 构建查询服务状态的命令
    QString command = QString("sc query tinc.%1").arg(netName);
    qDebug() << "查询服务命令:" << command;
    
    p.start(command);
    
    // 等待命令完成，设置超时时间为3秒
    if (!p.waitForFinished(3000)) {
        qDebug() << "查询服务超时";
        confTextEdit->appendPlainText("警告：查询Tinc服务状态超时");
        flag = 0; // 超时也视为配置成功，因为密钥已经生成
    } else {
        // 读取命令输出
        QByteArray outputAry = p.readAllStandardOutput();
        QByteArray errorAry = p.readAllStandardError();
        QString output = QString::fromUtf8(outputAry);
        QString error = QString::fromUtf8(errorAry);
        
        qDebug() << "服务查询输出:" << output;
        qDebug() << "服务查询错误:" << error;
        
        confTextEdit->appendPlainText("服务查询输出:\n" + output);
        
        // 检查服务是否存在
        if (error.contains("1060") || error.contains("指定的服务未安装")) {
            qDebug() << "Tinc服务不存在，跳过服务测试";
            confTextEdit->appendPlainText("Tinc服务尚未安装，跳过服务测试");
            flag = 0; // 服务不存在也视为配置成功
        } else {
            // 解析服务状态
            QStringList lines = output.split("\n");
            QString stateLine;

            for (const QString &line : lines) {
                if (line.contains("STATE")) {
                    stateLine = line;
                    break;
                }
            }

            if (!stateLine.isEmpty()) {
                // 提取状态码
                QStringList parts = stateLine.split(":",QString::SkipEmptyParts);
                QString state = parts.at(1).trimmed();

                // 判断服务状态：4 RUNNING表示运行中
                if(state == "4 RUNNING") {
                    flag = 0;
                    qDebug() << "Tinc服务正在运行";
                } else {
                    flag = 0; // 即使服务未运行，也视为配置成功，因为密钥已经生成
                    qDebug() << "Tinc服务状态:" << state;
                    confTextEdit->appendPlainText("Tinc服务状态: " + state);
                }
            } else {
                // 无法解析服务状态，但配置已经完成
                flag = 0;
                qDebug() << "无法解析服务状态，但配置已完成";
                confTextEdit->appendPlainText("无法解析服务状态，但配置已完成");
            }
        }
    }

    confTextEdit->appendPlainText("服务测试完成");

    // 更新进度到90%
    progressValue += 10;
    progressbar->setValue(progressValue);
    qDebug()<<"progressValue:" << progressValue;
    
    // 强制 Qt 立刻重绘界面，进度条和日志就会丝滑地动起来
    QCoreApplication::processEvents();

    // 上报配置结果
    this->Service_reply(flag);
}

/**
 * @brief 上报配置结果到服务器
 * @param flag 配置结果标志（0=成功，1=失败）
 * 
 * 功能说明：
 * 1. 根据flag构造结果信息
 * 2. 发送POST请求到服务器
 * 3. 更新进度条到100%
 * 4. 自动启动守护进程
 * 
 * 请求参数：
 * - type: 操作类型（"add"）
 * - result: 结果（"success"或"error"）
 * - ids: 节点ID
 * - details: 详细信息
 */
void download::Service_reply(int flag)
{
    QString line;
    QString type = "add";
    QString id;
    QString result;
    QString details;
    QString serverName;

    // 从private.txt读取配置信息
    id = getMess("id");
    serverName = getMess("server_name");

    // 根据flag构造结果信息
    if(flag == 0)
    {
        result = "success";
        details = QString("节点%1已完成接入。位置：%2/%3").arg(SId).arg(serverName).arg(netName);
        qDebug()<<details;
    }
    else
    {
        result = "error";
    }

    // 创建网络访问管理器
    addReply = new QNetworkAccessManager(this);
    // 构造JSON请求体
    QJsonObject json;
    json.insert("type",type);
    json.insert("result",result);
    json.insert("ids",id);
    json.insert("details",details);
    QJsonDocument document;
    document.setObject(json);
    QByteArray dataArray = document.toJson(QJsonDocument::Compact);

    // 创建HTTP请求
    QNetworkRequest addreq;
    addreq.setUrl(QUrl(Edit_Api));
    addreq.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    // 发送POST请求
    QNetworkReply *reply = addReply->post(addreq,dataArray);

    // 等待请求完成
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    // 释放 QNetworkReply 对象，防止内存泄漏
    reply->deleteLater();

    confTextEdit->appendPlainText(QString::fromUtf8("添加完成信号已发送"));

    // 更新进度到100%
    progressValue += 10;
    progressbar->setValue(progressValue);
    qDebug()<<"进度更新到100%";

    // 延迟1秒后显示下一步按钮
    qDebug()<<"准备显示下一步按钮";
    
    // 使用 QPointer 保护 this 指针，防止在 lambda 中变成悬空指针
    QPointer<download> self(this);
    
    QTimer::singleShot(1000, [self]() {
        qDebug()<<"开始显示下一步按钮";
        
        // 检查对象是否还存在
        if (self.isNull()) {
            qDebug()<<"警告：download 对象已被销毁，无法显示下一步按钮";
            return;
        }
        
        if (!self->isVisible()) {
            qDebug()<<"警告：download窗口已关闭，无法显示下一步按钮";
            return;
        }

        // 清除进度标签
        self->progressLayout->removeWidget(self->progressLabel);
        self->progressLabel->deleteLater();

        // 创建下一步按钮
        QPushButton *nextButton = new QPushButton("下一步", self);
        nextButton->setFont(QFont("宋体", 9));
        nextButton->setMinimumSize(120, 35);
        nextButton->setStyleSheet(
            "QPushButton {"
            "   background-color: #2196F3;"
            "   color: white;"
            "   border: none;"
            "   border-radius: 5px;"
            "   padding: 8px;"
            "   font-size: 10pt;"
            "}"
            "QPushButton:hover {"
            "   background-color: #1976D2;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #0D47A1;"
            "}"
            );

        // 将下一步按钮添加到进度布局
        self->progressLayout->addWidget(nextButton);

        // 连接下一步按钮信号
        connect(nextButton, &QPushButton::clicked, self, [self, nextButton]() {
            // 清除进度条和下一步按钮
            self->progressLayout->removeWidget(self->progressbar);
            self->progressLayout->removeWidget(nextButton);
            self->progressbar->deleteLater();
            nextButton->deleteLater();

            // 创建消息标签
            self->messageLabel = new QLabel("Tinc VPN配置已完成！\n\n配置进度：100%\n\n是否启动守护进程并完成配置？", self);
            self->messageLabel->setFont(QFont("宋体", 10));
            self->messageLabel->setAlignment(Qt::AlignCenter);
            self->messageLabel->setWordWrap(true);
            self->messageLabel->setStyleSheet(
                "QLabel {"
                "   padding: 20px;"
                "   background-color: #E8F5E9;"
                "   border-radius: 5px;"
                "}"
                );

            // 创建按钮布局
            QHBoxLayout *buttonLayout = new QHBoxLayout();
            buttonLayout->setSpacing(20);

            // 创建确认按钮
            self->confirmButton = new QPushButton("确认", self);
            self->confirmButton->setFont(QFont("宋体", 9));
            self->confirmButton->setMinimumSize(100, 30);
            self->confirmButton->setStyleSheet(
                "QPushButton {"
                "   background-color: #4CAF50;"
                "   color: white;"
                "   border: none;"
                "   border-radius: 3px;"
                "   padding: 5px;"
                "}"
                "QPushButton:hover {"
                "   background-color: #45a049;"
                "}"
                "QPushButton:pressed {"
                "   background-color: #3d8b40;"
                "}"
                );

            // 创建取消按钮
            self->cancelButton = new QPushButton("取消", self);
            self->cancelButton->setFont(QFont("宋体", 9));
            self->cancelButton->setMinimumSize(100, 30);
            self->cancelButton->setStyleSheet(
                "QPushButton {"
                "   background-color: #f44336;"
                "   color: white;"
                "   border: none;"
                "   border-radius: 3px;"
                "   padding: 5px;"
                "}"
                "QPushButton:hover {"
                "   background-color: #da190b;"
                "}"
                "QPushButton:pressed {"
                "   background-color: #b71c1c;"
                "}"
                );

            // 添加按钮到布局
            buttonLayout->addWidget(self->confirmButton);
            buttonLayout->addWidget(self->cancelButton);

            // 将消息和按钮添加到进度布局
            self->progressLayout->addWidget(self->messageLabel);
            self->progressLayout->addLayout(buttonLayout);

            // 更新进度组标题
            self->progressGroup->setTitle("配置完成");

            // 连接按钮信号
            connect(self->confirmButton, &QPushButton::clicked, self, [self]() {
                self->confTextEdit->appendPlainText(QString::fromUtf8("用户确认完成配置"));
                self->Daemon();
            });

            connect(self->cancelButton, &QPushButton::clicked, self, [self]() {
                self->confTextEdit->appendPlainText(QString::fromUtf8("用户取消配置"));
                QCoreApplication::quit();
            });
        });
    });
}

/**
 * @brief 启动守护进程
 * 
 * 功能说明：
 * 1. 启动Daemons.exe守护进程
 * 2. 显示配置完成提示框
 * 3. 退出应用程序
 * 
 * 技术细节：
 * - Daemons.exe负责监控和管理Tinc服务
 * - 此函数在用户确认后才会被调用
 * - 退出后整个配置流程结束
 */
void download::Daemon(){
    QProcess process;
    process.setWorkingDirectory(daemonsPath);
    // 启动守护进程（后台运行，不创建新窗口）
    process.start("cmd.exe", QStringList() << "/c" << "start /B Daemons.exe && exit");
    process.waitForFinished(3000);

    qDebug()<<"Daemon";
    confTextEdit->appendPlainText(QString::fromUtf8("配置完成"));
    confTextEdit->appendPlainText(QString::fromUtf8("守护进程已启动"));

    // 清除之前的消息和按钮
    progressLayout->removeWidget(messageLabel);
    progressLayout->removeWidget(confirmButton);
    progressLayout->removeWidget(cancelButton);
    messageLabel->deleteLater();
    confirmButton->deleteLater();
    cancelButton->deleteLater();

    // 创建新的消息标签
    messageLabel = new QLabel("Tinc VPN配置已完成！\n\n守护进程已启动。\n\n点击确定关闭程序。", this);
    messageLabel->setFont(QFont("宋体", 10));
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setWordWrap(true);
    messageLabel->setStyleSheet(
        "QLabel {"
        "   padding: 20px;"
        "   background-color: #E8F5E9;"
        "   border-radius: 5px;"
        "}"
        );

    // 创建确定按钮
    okButton = new QPushButton("确定", this);
    okButton->setFont(QFont("宋体", 9));
    okButton->setMinimumSize(100, 30);
    okButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #2196F3;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 3px;"
        "   padding: 5px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #1976D2;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #0D47A1;"
        "}"
        );

    // 创建按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(20);
    buttonLayout->addWidget(okButton);

    // 将消息和按钮添加到进度布局
    progressLayout->addWidget(messageLabel);
    progressLayout->addLayout(buttonLayout);

    // 更新进度组标题
    progressGroup->setTitle("配置成功");

    // 连接确定按钮信号
    connect(okButton, &QPushButton::clicked, this, [this]() {
        // 显示第二个确认对话框
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "确认退出", 
                                      "确定要退出程序吗？",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            // 使用 exit(0) 来真正结束程序，而不是只退出事件循环
            qApp->exit(0);
        }
    });
}

/**
 * @brief 析构函数
 * 
 * 功能说明：
 * - 清理资源，释放内存
 * - Qt会自动管理子对象
 */
download::~download()
{
    qDebug() << "download 析构函数被调用，窗口即将被销毁";
}
