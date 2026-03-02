#ifndef CONF_PACK_H
#define CONF_PACK_H

#include <QMainWindow>
#include <QDebug>
#include <QProcess>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>

/**
 * @brief Tinc VPN配置和密钥管理类
 * 
 * 类功能说明：
 * 1. 负责生成RSA密钥对（公钥和私钥）
 * 2. 管理Tinc VPN的配置文件
 * 3. 将客户端公钥上传到服务器
 * 4. 处理与服务器端的通信
 * 
 * 工作流程：
 * 1. 构造函数初始化参数并启动密钥生成
 * 2. generate_key()生成RSA密钥对
 * 3. uploadPublicKey()将公钥上传到服务器
 * 4. onUploadFinished()处理上传结果并退出
 * 
 * 技术特点：
 * - 使用QProcess调用tincd.exe命令行工具
 * - 使用QNetworkAccessManager进行HTTP通信
 * - 支持JSON格式的数据交换
 * - 自动化的密钥生成和上传流程
 */
class conf_package : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
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
     * - 初始化网络访问管理器
     * - 连接信号槽机制
     * - 自动启动密钥生成流程
     */
    conf_package(const QString& tem_SId,const QString& tem_token,const QString& tem_netName,const QString& tem_nodeIp,const QString& tem_action,const QString& tem_key,const QString& tem_value,const QString& tem_serverIp,QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     * 
     * 功能说明：
     * - 清理资源，释放内存
     * - Qt会自动管理子对象
     */
    ~conf_package();

    /**
     * @brief 节点ID（用户名/客户端标识符）
     * 
     * 用途：
     * - 标识客户端身份
     * - 作为公钥文件名
     * - 服务器端识别客户端
     */
    QString SId;
    
    /**
     * @brief 认证令牌
     * 
     * 用途：
     * - 验证客户端身份
     * - 确保请求合法性
     * - 防止未授权访问
     */
    QString token;
    
    /**
     * @brief 网络名称
     * 
     * 用途：
     * - 指定Tinc VPN网络
     * - 确定配置文件目录
     * - 区分不同的VPN网络
     */
    QString netName;
    
    /**
     * @brief 节点IP地址
     * 
     * 用途：
     * - 分配给客户端的虚拟IP
     * - VPN网络中的通信地址
     * - 配置文件中的Subnet参数
     */
    QString nodeIp;
    
    /**
     * @brief 操作类型
     * 
     * 用途：
     * - 指定对节点的操作类型
     * - "add"表示添加节点
     * - "delete"表示删除节点
     * - "update"表示更新节点
     */
    QString action;
    
    /**
     * @brief 配置键（预留参数）
     * 
     * 用途：
     * - 预留用于扩展配置项
     * - 当前未使用
     */
    QString key;
    
    /**
     * @brief 配置值（预留参数）
     * 
     * 用途：
     * - 预留用于扩展配置值
     * - 当前未使用
     */
    QString value;
    
    /**
     * @brief 服务器IP地址
     * 
     * 用途：
     * - 指定上传公钥的服务器地址
     * - 从private.txt读取
     * - 用于构建上传URL
     */
    QString serverIp;

    /**
     * @brief 网络访问管理器
     * 
     * 用途：
     * - 发送HTTP请求
     * - 处理网络通信
     * - 管理网络连接
     */
    QNetworkAccessManager* manager;

public slots:
    /**
     * @brief 生成RSA密钥对
     * 
     * 功能说明：
     * 1. 检查并创建必要的目录结构
     * 2. 调用tincd.exe生成密钥对
     * 3. 验证密钥生成是否成功
     * 4. 成功后自动触发公钥上传
     * 
     * 技术细节：
     * - 使用QProcess调用tincd.exe
     * - 参数 -n 指定网络名称
     * - 参数 -K 表示生成密钥
     * - 生成rsa_key.priv和hosts/<节点名>文件
     */
    void generate_key();
    
    /**
     * @brief 上传公钥到服务器
     * 
     * 功能说明：
     * 1. 读取生成的公钥文件
     * 2. 构造JSON格式的HTTP请求
     * 3. 发送POST请求到服务器
     * 4. 等待服务器响应
     * 
     * 请求参数：
     * - sid：节点ID
     * - token：认证令牌
     * - netName：网络名称
     * - nodeIp：节点IP地址
     * - action：操作类型
     * - content：公钥内容
     */
    void uploadPublicKey();
    
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
     */
    void onUploadFinished(QNetworkReply* reply);
};

#endif // CONF_PACK_H
