# Daemons 模块说明书

## 模块作用
`Daemons` 模块是 Tinc VPN 客户端的核心后台服务，基于 `QtService` 实现，作为 Windows 服务运行。它负责维持客户端与服务器的实时通信、状态监控及配置动态更新。

### 主要功能：
1. **心跳维持**：定期发送心跳包（Heartbeat）到服务器，报告在线状态并拉取更新指令。
2. **动态配置更新**：实时处理服务器下发的配置修改指令（如修改节点 IP 或节点名称），并自动触发重新配置流程。
3. **异常监控与自愈**：通过 `ping` 网关 IP 监控 VPN 隧道连通性，并在异常时检查配置文件完整性及服务运行状态。
4. **服务状态管理**：监控 `tinc.<netName>` 服务的运行状态（Running/Stopped）。

---

## 关键代码说明

### 1. 服务生命周期管理 (`start`)
初始化环境、读取本地私有配置并启动定时任务。

```cpp
void DaemonService::start() {
    sid = getMess("sid");
    netName = getMess("net_name");
    // ... 初始化 API 地址 ...
    
    // 启动定时器：1秒一次心跳，10秒一次连通性检查
    this->setTime();
}
```

### 2. 动态更新逻辑 (`HeartReply`)
解析心跳响应，如果收到更新指令（非 "1" 响应），则执行文件修改和重新配置。

```cpp
void DaemonService::HeartReply(QNetworkReply* reply) {
    if(reply->error() == QNetworkReply::NoError){
        QString response = QString::fromUtf8(reply->readAll());
        if(response == "1") return; // 正常
        
        // 处理更新指令 (例如 "Subnet：10.0.0.5")
        QStringList update_list = response.split("：");
        if(update_list[0] == "Subnet"){
            // 1. 修改本地 hosts 节点文件
            modifyFileContent(node_FilePath, "Subnet", newValue);
            // 2. 删除旧私钥，触发重新生成
            deleteFile(host_path, "rsa_key.priv");
            // 3. 调用 conf_package 重新生成密钥并上传
            conf_pack("", nodeIp);
        }
    }
}
```

### 3. 网络连通性自检 (`ping`)
周期性检测网关可达性，判定 VPN 隧道是否正常。

```cpp
void DaemonService::ping() {
    QProcess process;
    process.start("ping", QStringList() << gatewayIp);
    process.waitForFinished();
    QString outPut = process.readAll();

    if(outPut.contains("不可达")) {
        // 发现异常，停止心跳并检查本地配置文件和 Service 状态
        timer->stop();
        checkServiceStatus();
    }
}
```

---

## 技术细节
- **服务框架**：使用 `QtService<QCoreApplication>` 实现 Windows 服务包装。
- **配置同步**：通过修改 `private.txt` 和调用外部工具 `conf_package.exe` 完成配置闭环。
- **进程调用**：使用 `QProcess` 执行 `sc query` (服务查询) 和 `tasklist` (进程查询)。
