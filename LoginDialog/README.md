# LoginDialog 模块说明书

## 模块作用
`LoginDialog` 模块是 Tinc VPN 客户端的用户前端入口，负责身份验证、环境初始化以及后续配置界面的导航。

### 主要功能：
1. **身份认证界面**：提供用户名（SID）和密码输入界面，具备现代化的 UI 设计（渐变按钮、阴影效果）。
2. **交互增强**：包含一个动态角色（AnimatedCharacter），能根据鼠标位置追踪视线，并在输入密码时“蒙眼”，提升用户体验。
3. **配置初始化**：登录成功后，从服务器获取 `Token`、`net_name` 及分配的虚拟 IP (`node_ip`)。
4. **私有数据持久化**：将登录成功的会话信息写入本地 `private.txt` 文件，作为其他模块（如 Daemons）的配置来源。
5. **界面流转**：关闭登录窗体并调起 `confg` 配置主界面。

---

## 关键代码说明

### 1. 登录请求发送 (`login`)
收集 UI 输入并向服务器发送 JSON 格式的验证请求。

```cpp
void Logindialog::login() {
    sid = this->uN->text().remove(' ');
    Password = this->pw->text().remove(' ');

    QNetworkRequest request(QUrl(Login_Api));
    QJsonObject json;
    json.insert("sid", sid);
    json.insert("password", Password);

    QJsonDocument document(json);
    manager->post(request, document.toJson());
}
```

### 2. 登录结果处理与文件写入 (`getBack`)
处理服务器响应，解析 JSON 并保存至 `private.txt`。

```cpp
void Logindialog::getBack(QNetworkReply *reply) {
    QByteArray data = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
    QJsonObject jsonObj = jsonDoc.object();

    if(jsonObj.value("status").toInt() == 1) { // 登录成功
        QFile priFile(parentpath + "/private.txt");
        if(priFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&priFile);
            stream << "sid:" << jsonObj.value("sid").toString() << "\n";
            stream << "token:" << jsonObj.value("token").toString() << "\n";
            stream << "net_name:" << jsonObj.value("net_name").toString() << "\n";
            stream << "node_ip:" << jsonObj.value("node_ip").toString() << "\n";
            priFile.close();
        }
        
        // 触发界面流转
        emit closed();
        this->close();
        m_confg = new confg(sid, Token, serverIp);
        m_confg->show();
    }
}
```

### 3. UI 交互：视线追踪与蒙眼
利用 `eventFilter` 捕捉焦点变化，实现趣味交互。

```cpp
bool Logindialog::eventFilter(QObject *obj, QEvent *event) {
    if (obj == pw) { // 密码框
        if (event->type() == QEvent::FocusIn) {
            character->setCoverEyes(true); // 蒙眼
        } else if (event->type() == QEvent::FocusOut) {
            character->setCoverEyes(false); // 取消蒙眼
        }
    }
    return QWidget::eventFilter(obj, event);
}
```

---

## UI 特色
- **无边框窗体**：使用 `Qt::FramelessWindowHint` 自定义标题栏逻辑。
- **自定义绘图**：在 `paintEvent` 中手动绘制多层路径实现的软阴影效果。
- **QSS 样式表**：高度定制化的样式，包括线性渐变背景和圆角输入框。
