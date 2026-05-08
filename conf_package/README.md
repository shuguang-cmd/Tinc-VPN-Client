# conf_package 模块说明书

## 模块作用
`conf_package` 是一个用于 Tinc VPN 配置和密钥管理的自动化工具模块。其核心职责是为客户端生成唯一的 RSA 密钥对，并将生成的公钥及相关节点信息（如 Subnet/虚拟IP）上传至中心服务器，以完成 VPN 节点的注册与互联准备。

### 主要功能：
1. **自动化目录初始化**：自动创建 Tinc 网络所需的目录结构（网络目录及 hosts 子目录）。
2. **RSA 密钥对生成**：通过调用 `tincd.exe -K` 命令行工具生成私钥 (`rsa_key.priv`) 和公钥文件。
3. **配置文件注入**：在生成的公钥文件中自动注入分配的 `Subnet` (虚拟 IP) 信息。
4. **远程公钥分发**：将公钥内容通过 HTTP 接口上传至服务器，供其他节点同步。

---

## 关键代码说明

### 1. 密钥生成与交互处理 (`generate_key`)
由于 `tincd -K` 是交互式命令，该函数通过 `QProcess` 自动注入两次回车来完成非交互式密钥生成。

```cpp
void conf_package::generate_key(){
    // ... 目录检查与旧密钥清理 ...
    QProcess p;
    p.setWorkingDirectory(workDir);
    p.start(tincdPath, QStringList() << "-n" << my_netName << "-K");
    
    // 注入两次回车并关闭写入通道，防止进程阻塞
    p.write("\n\n"); 
    p.closeWriteChannel(); 
    
    if (p.waitForFinished(15000)) {
        // 生成成功后注入 Subnet
        QFile hostFile(oldPubKey);
        if (hostFile.open(QIODevice::ReadWrite | QIODevice::Text)) {
            QTextStream out(&hostFile);
            out << "Subnet = " << nodeIp << "/32\n\n"; 
            // ...
        }
    }
}
```

### 2. 公钥上传 (`uploadPublicKey`)
使用 JSON 格式将节点信息和公钥内容打包上传。

```cpp
void conf_package::uploadPublicKey(){
    // 构建 JSON 请求体
    QJsonObject json;
    json.insert("sid", SId);
    json.insert("token", token);
    json.insert("netName", netName);
    json.insert("nodeIp", nodeIp);
    json.insert("action", "add");
    json.insert("content", publicKeyContent); // 读取自 hosts/<SId>

    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->post(request, dataArray);
}
```

---

## 工作流程
1. **构造初始化**：接收 `SId` (用户ID), `token`, `nodeIp` 等参数。
2. **密钥创建**：调用 `generate_key()`，通过 `QProcess` 与 `tincd` 交互。
3. **内容修正**：向 `hosts` 文件写入 `Subnet` 参数。
4. **网络同步**：执行 `uploadPublicKey()`，成功后通过 `onUploadFinished` 回调退出程序。
