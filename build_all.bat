@echo off
setlocal EnableDelayedExpansion

:: ==========================================
:: -1. 自动获取管理员权限
:: ==========================================
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"
if '%errorlevel%' NEQ '0' (
    echo [INFO] Requesting Administrative Privileges...
    goto UACPrompt
) else ( goto gotAdmin )

:UACPrompt
    echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"
    echo UAC.ShellExecute "%~s0", "", "", "runas", 1 >> "%temp%\getadmin.vbs"
    "%temp%\getadmin.vbs"
    exit /B

:gotAdmin
    if exist "%temp%\getadmin.vbs" ( del "%temp%\getadmin.vbs" )
    pushd "%CD%"
    CD /D "%~dp0"

:: ==========================================
:: 0. 强制关闭正在运行的程序
:: ==========================================
echo [INFO] Killing running processes to release file locks...
taskkill /F /IM LoginDialog.exe /T >nul 2>&1
taskkill /F /IM Daemons.exe /T >nul 2>&1
taskkill /F /IM conf_package.exe /T >nul 2>&1
timeout /t 1 /nobreak >nul
echo [INFO] Old processes killed. Ready to build.
echo.

:: ==========================================
:: 1. 设置 Qt 环境路径
:: ==========================================
set QT_BIN=D:\ALLApp\Qt\5.15.2\mingw81_64\bin
set MINGW_BIN=D:\ALLApp\Qt\Tools\mingw810_64\bin
set PATH=%QT_BIN%;%MINGW_BIN%;%PATH%

set SOURCE_ROOT=%~dp0
set DEPLOY_ROOT=%SOURCE_ROOT%..\demo_setup_win

echo [INFO] Environment setup complete.
echo [INFO] Deploy Root: %DEPLOY_ROOT%
echo.

:: ==========================================
:: 2. 定义编译函数
:: ==========================================
goto :StartBuild

:BuildProject
set PROJ_NAME=%~1
echo ==========================================
echo [BUILDING] %PROJ_NAME% ...
echo ==========================================
:: 🔴 核心修复：加上 /d 强制跨盘符切换
cd /d "%SOURCE_ROOT%%PROJ_NAME%"

if exist Makefile (
    mingw32-make clean >nul 2>&1
)

qmake %PROJ_NAME%.pro -spec win32-g++ "CONFIG+=release"

mingw32-make -j8
if %errorlevel% neq 0 (
    echo [ERROR] Compilation failed for %PROJ_NAME%!
    pause
    exit /b %errorlevel%
)

echo [COPYING] %PROJ_NAME%.exe to deployment folder...
copy /Y "release\%PROJ_NAME%.exe" "%DEPLOY_ROOT%\%PROJ_NAME%\"

echo [SUCCESS] %PROJ_NAME% built and deployed.
echo.
exit /b 0

:: ==========================================
:: 3. 开始执行任务
:: ==========================================
:StartBuild
call :BuildProject LoginDialog
call :BuildProject Daemons
call :BuildProject conf_package

echo ==========================================
echo       ALL BUILDS SUCCESSFUL!
echo ==========================================
pause