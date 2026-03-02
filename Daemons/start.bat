@echo off
cd /d D:\Users\laboratory\qt\build-Daemons-Desktop_Qt_5_12_2_MinGW_32_bit-Debug\debug

REM 创建服务
sc create DaemonService binPath= "D:\Users\laboratory\qt\build-Daemons-Desktop_Qt_5_12_2_MinGW_32_bit-Debug\debug\Daemons.exe"

REM 配置服务为自动启动
sc config DaemonService start= auto

REM 启动服务
net start DaemonService

echo 服务启动完毕！
pause
