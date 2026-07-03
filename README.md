# Tinc VPN 客户端配置工具 (TMG)

基于 Qt 5.15.2 开发的 Tinc VPN 图形化管理客户端，提供用户登录、配置文件下载、RSA 密钥对自动生成、公钥上传及服务器注册等功能。目标平台为 Windows（MinGW 8.1.0）。

## 功能概览

- **用户登录与身份验证**：通过后端 API 验证用户身份，获取认证令牌。
- **配置自动下载**：从服务器拉取 Tinc 配置文件压缩包并解压到本地。
- **RSA 密钥自动生成**：调用 tincd.exe 生成节点密钥对。
- **公钥上传与注册**：将公钥通过 HTTP POST 上报至服务端，完成节点注册。
- **配置进度可视化**：向导界面实时展示配置进度（0% → 100%）。
- **日志记录**：`conf_package` 模块输出 `conf_log.txt`，便于排查问题。
- **后台守护进程**：以 Windows 服务形式托管 Tinc 进程，支持自动重启。

## 项目结构

```
code_win/
├── LoginDialog/              # 登录与配置界面模块
│   ├── logindialog.cpp/h     #   登录窗口
│   ├── download.cpp/h        #   配置下载逻辑
│   ├── confg.cpp/h           #   配置向导界面
│   ├── background.cpp/h      #   自定义背景/动画
│   ├── AnimatedCharacter.h   #   动态小人控件
│   ├── test.cpp/h            #   测试工具
│   ├── uac.h                 #   UAC 权限提升
│   ├── res.h                 #   内嵌资源
│   ├── Icon/                 #   图标资源
│   ├── res/                  #   资源文件
│   ├── main.cpp              #   入口
│   └── LoginDialog.pro       #   Qt 项目文件
├── conf_package/             # 密钥生成与公钥上传模块
│   ├── conf_package.cpp/h    #   核心逻辑
│   ├── main.cpp              #   入口
│   └── conf_package.pro      #   Qt 项目文件
├── Daemons/                  # 后台守护进程模块
│   ├── DaemonServer.cpp/h    #   服务核心
│   ├── Daemons.h             #   公共头文件
│   ├── main.cpp              #   入口
│   ├── qtservice/            #   QtService 框架
│   └── Daemons.pro           #   Qt 项目文件
├── build_all.bat             # 一键编译脚本（含管理员权限提升）
├── deploy.bat                # Qt 依赖部署脚本
└── README.md
```

## 环境要求

| 组件 | 版本 / 说明 |
|---|---|
| 操作系统 | Windows 7 / 10 / 11 |
| Qt | 5.15.2 (Core, Network, Widgets, Gui) |
| 编译器 | MinGW 8.1.0 (mingw81_64) |
| Tinc VPN | 1.1+（需提供 `tincd.exe`） |
| TAP 驱动 | tap-win64（包含在 Tinc 安装目录中） |

## 快速开始

### 1. 克隆仓库

```bash
git clone <repo-url>
cd tinc_cli_gui/windows/demo_win/code_win
```

### 2. 编译

**方式一：一键编译（推荐）**

以管理员身份运行 `build_all.bat`，脚本会自动：
- 结束正在运行的旧进程
- 依次编译 LoginDialog、Daemons、conf_package
- 将产物拷贝到 `../demo_setup_win` 并运行 `windeployqt`

**方式二：手动逐个编译**

```bash
# 设置环境变量
set QT_BIN=D:\ALLApp\Qt\5.15.2\mingw81_64\bin
set MINGW_BIN=D:\ALLApp\Qt\Tools\mingw810_64\bin
set PATH=%QT_BIN%;%MINGW_BIN%;%PATH%

# 编译 LoginDialog
cd LoginDialog
qmake LoginDialog.pro -spec win32-g++ "CONFIG+=release"
mingw32-make -j8

# 编译 Daemons
cd ../Daemons
qmake Daemons.pro -spec win32-g++ "CONFIG+=release"
mingw32-make -j8

# 编译 conf_package
cd ../conf_package
qmake conf_package.pro -spec win32-g++ "CONFIG+=release"
mingw32-make -j8
```

### 3. 部署依赖

```bash
deploy.bat
```

该脚本对 `bin/` 目录下的三个可执行文件调用 `windeployqt`，自动拉取所需的 Qt 动态库。

### 4. 运行

```bash
# 启动登录界面
cd bin
LoginDialog.exe
```

## 工作流程

```
用户登录 → 下载配置 → 生成密钥 → 上传公钥 → 配置完成 → 启动 VPN
   ↓          ↓          ↓          ↓          ↓
 获取Token  解压ZIP   调用tincd    POST到      守护进程
                      生成RSA      服务器      接管运行
```

### 配置进度说明

| 进度 | 阶段 |
|---|---|
| 0% | 初始化完成，开始下载 |
| 10% | 配置文件下载完成 |
| 20% | 开始生成密钥 |
| 50% | 密钥生成完成 |
| 80% | 开始上传公钥 |
| 100% | 配置完成 |

## 守护进程管理

Daemons 模块基于 QtService 实现，可注册为 Windows 服务：

```bash
Daemons.exe -i    # 安装服务
Daemons.exe -s    # 启动服务
Daemons.exe -t    # 停止服务
Daemons.exe -u    # 卸载服务
```

## 配置文件格式

### private.txt

位于 `Tinc/` 安装目录下，供 `conf_package` 读取：

```
net_name:Hello
server_ip:127.0.0.1:8081
node_ip:192.168.103.34
```

| 字段 | 说明 |
|---|---|
| `net_name` | Tinc 网络名称 |
| `server_ip` | 服务端 IP（含端口） |
| `node_ip` | 本节点内网 IP |

## API 接口

### 登录

```
POST /login
{ "username": "...", "password": "..." }
→ { "code": 200, "token": "sun-token-xxx" }
```

### 下载配置

```
POST /downloadConfig
{ "sid": "节点ID", "token": "认证令牌" }
→ ZIP 压缩包（二进制）
```

### 上传公钥

```
POST /XVntQFJCjc.php/coreplugs/Clientinterface/exchangeFile
{ "sid": "...", "token": "...", "netName": "...", "nodeIp": "...", "action": "add", "content": "-----BEGIN RSA PUBLIC KEY-----..." }
→ 服务端公钥信息
```

## 常见问题

| 问题 | 排查方向 |
|---|---|
| 无法连接服务器 | 检查网络、防火墙、`private.txt` 中的 IP 和端口 |
| 密钥生成失败 | 确认 `tincd.exe` 路径正确、有写入权限 |
| 公钥上传失败 | 检查 Token 是否过期、服务端是否正常运行 |
| VPN 无法启动 | 检查 TAP 驱动是否安装、配置文件是否正确 |
| 查看日志 | 客户端日志 `conf_log.txt`；Tinc 日志 `Tinc/<网络名>/log.txt` |

## 技术栈

- **框架**: Qt 5.15.2
- **语言**: C++ (C++11)
- **编译器**: MinGW 8.1.0 (Windows)
- **VPN**: Tinc 1.1
- **协议**: HTTP/HTTPS

## 版本历史

### v1.0.0

- 用户登录与 Token 认证
- 配置文件自动下载与解压
- RSA 密钥对自动生成
- 公钥上传与服务器注册
- 配置进度可视化
- Windows 服务守护进程
- 界面美化（自定义背景、动态小人）
- 修复子进程路径问题
- 修复 Subnet 属性提取错误
- 修复密钥叠加问题
- 修复进度条到达 100% 后退出与按钮显示问题

## 许可证

MIT License

## 参考资料

- [Tinc VPN 官方文档](https://www.tinc-vpn.org/documentation/)
- [Qt 5.15 文档](https://doc.qt.io/qt-5/)
