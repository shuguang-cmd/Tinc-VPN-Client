@echo off
chcp 65001
echo ==========================================
echo    Tinc VPN 客户端工业级打包部署脚本
echo ==========================================

echo [1/3] 配置 Qt 环境变量...
set PATH=D:\ALLApp\Qt\5.15.2\mingw81_64\bin;%PATH%

echo [2/3] 进入统一输出目录 bin\...
cd /d "%~dp0\bin"
if %errorlevel% neq 0 (
    echo [错误] 找不到 bin 目录，请确保已在 Release 模式下编译所有工程！
    pause
    exit /b 1
)

echo [3/3] 开始疯狂抓取依赖库...

echo    - 部署 LoginDialog.exe...
windeployqt --release LoginDialog.exe

echo    - 部署 conf_package.exe...
windeployqt --release conf_package.exe

echo    - 部署 Daemons.exe...
windeployqt --release Daemons.exe

echo [4/4] 拷贝 OpenSSL 动态库（HTTPS 支持）...
set OPENSSL_BIN=D:\ALLApp\Qt\Tools\OpenSSL\Win_x64\bin
if not exist "%OPENSSL_BIN%\libssl-1_1-x64.dll" (
    echo [警告] 未找到 OpenSSL，跳过拷贝。路径: %OPENSSL_BIN%
    echo        HTTPS 请求将无法正常工作！
    goto :done
)
copy /Y "%OPENSSL_BIN%\libssl-1_1-x64.dll"    .
copy /Y "%OPENSSL_BIN%\libcrypto-1_1-x64.dll" .
echo    - libssl-1_1-x64.dll    已拷贝
echo    - libcrypto-1_1-x64.dll 已拷贝

:done
echo ==========================================
echo 部署完成！bin 目录现已具备免安装绿色版能力。
echo ==========================================
pause