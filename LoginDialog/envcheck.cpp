#include "envcheck.h"

#include <QAbstractButton>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDebug>

// ============================================================
// 构造函数
// ============================================================
EnvCheck::EnvCheck(QWidget *parent)
    : QObject(parent), m_parent(parent)
{
    resolvePaths();
}

// ============================================================
// 路径解析：兼容开发模式与部署模式
//
//  开发模式：exe 在 LoginDialog/build/debug/ → cdUp x3 → code_win/
//  部署模式：exe 在 bin/                      → cdUp x1 → code_win/
// ============================================================
void EnvCheck::resolvePaths()
{
    QDir appDir(QCoreApplication::applicationDirPath());

    // 尝试开发模式路径（3 级向上）
    QDir devDir = appDir;
    devDir.cdUp(); // debug -> build
    devDir.cdUp(); // build -> LoginDialog
    devDir.cdUp(); // LoginDialog -> code_win

    // 尝试部署模式路径（1 级向上）
    QDir prodDir = appDir;
    prodDir.cdUp(); // bin -> code_win

    // 选择实际存在 tincd.exe 的路径
    if (QFile::exists(devDir.absoluteFilePath("Tinc/tincd.exe"))) {
        m_tincDir = devDir.absoluteFilePath("Tinc");
    } else {
        // 默认使用部署模式路径（即使 tincd 不存在，checkTincd 会给出提示）
        m_tincDir = prodDir.absoluteFilePath("Tinc");
    }

    m_tapDir = m_tincDir + "/tap-win64";

    qDebug() << "[EnvCheck] Tinc 目录:" << m_tincDir;
    qDebug() << "[EnvCheck] TAP  目录:" << m_tapDir;
}

// ============================================================
// 主入口：依次执行所有检查
// ============================================================
bool EnvCheck::runChecks()
{
    if (!checkTincd())    return false; // 致命：tincd 不存在则无法生成密钥
    if (!checkTapDriver()) return false; // 致命：用户拒绝且确认不继续
    return true;
}

// ============================================================
// 检查 tincd.exe 是否存在
// ============================================================
bool EnvCheck::checkTincd()
{
    QString tincdPath = m_tincDir + "/tincd.exe";
    if (QFile::exists(tincdPath)) {
        qDebug() << "[EnvCheck] tincd.exe 检查通过:" << tincdPath;
        return true;
    }

    qDebug() << "[EnvCheck] tincd.exe 未找到:" << tincdPath;

    QMessageBox::critical(m_parent,
        "环境缺失 — 无法启动",
        QString(
            "未找到 Tinc 核心程序：\n\n"
            "  %1\n\n"
            "请联系管理员获取完整部署包，\n"
            "将 tincd.exe 放置到上述路径后重新运行。"
        ).arg(tincdPath)
    );
    return false;
}

// ============================================================
// 检查 TAP 网卡驱动是否已安装
// ============================================================
bool EnvCheck::checkTapDriver()
{
    // 使用 sc query 检查 tap0901 服务（TAP-Windows 驱动注册的服务名）
    QProcess p;
    p.start("sc", QStringList() << "query" << "tap0901");
    p.waitForFinished(5000);

    QString output = QString::fromLocal8Bit(p.readAllStandardOutput());
    Q_UNUSED(p.readAllStandardError()); // 仅用于清空缓冲区
    qDebug() << "[EnvCheck] sc query tap0901 输出:" << output;

    // 输出中包含 STATE 说明服务已注册，即驱动已安装
    if (output.contains("STATE")) {
        qDebug() << "[EnvCheck] TAP 驱动已安装";
        return true;
    }

    // 驱动未安装，询问用户
    qDebug() << "[EnvCheck] TAP 驱动未安装，提示用户";

    QMessageBox msgBox(m_parent);
    msgBox.setWindowTitle("检测到 TAP 网卡未安装");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(
        "Tinc VPN 需要 TAP 虚拟网卡驱动才能正常工作。\n\n"
        "当前系统未检测到 TAP 驱动（tap0901）。"
    );
    msgBox.setInformativeText(
        "是否立即自动安装 TAP 网卡驱动？\n"
        "（需要管理员权限，部分情况下安装后需要重启）"
    );
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Yes);
    msgBox.button(QMessageBox::Yes)->setText("立即安装");
    msgBox.button(QMessageBox::No)->setText("跳过（VPN 可能无法连接）");
    msgBox.button(QMessageBox::Cancel)->setText("退出程序");

    int result = msgBox.exec();

    if (result == QMessageBox::Yes) {
        return installTapDriver();
    } else if (result == QMessageBox::No) {
        QMessageBox::warning(m_parent, "警告",
            "已跳过 TAP 驱动安装。\n\n"
            "配置流程可以继续，但 VPN 最终可能无法建立连接。\n"
            "如需安装，请以管理员身份重新运行程序。");
        return true; // 允许继续（用户自愿承担风险）
    } else {
        // 用户选择退出
        return false;
    }
}

// ============================================================
// 安装 TAP 网卡驱动
// ============================================================
bool EnvCheck::installTapDriver()
{
    QString tapInstall = m_tapDir + "/tapinstall.exe";
    QString infFile    = m_tapDir + "/OemVista.inf";

    // 检查安装程序是否存在
    if (!QFile::exists(tapInstall)) {
        QMessageBox::critical(m_parent, "安装失败",
            QString(
                "未找到 TAP 驱动安装程序：\n\n"
                "  %1\n\n"
                "请联系管理员获取包含 tap-win64 目录的完整部署包。"
            ).arg(tapInstall)
        );
        return false;
    }

    if (!QFile::exists(infFile)) {
        QMessageBox::critical(m_parent, "安装失败",
            QString(
                "未找到 TAP 驱动配置文件：\n\n"
                "  %1\n\n"
                "tap-win64 目录可能不完整，请联系管理员。"
            ).arg(infFile)
        );
        return false;
    }

    qDebug() << "[EnvCheck] 开始安装 TAP 驱动...";
    qDebug() << "[EnvCheck] tapinstall.exe:" << tapInstall;
    qDebug() << "[EnvCheck] inf 文件:" << infFile;

    QProcess p;
    p.setWorkingDirectory(m_tapDir);
    p.start(tapInstall, QStringList() << "install" << infFile << "tap0901");

    if (!p.waitForFinished(30000)) {
        p.kill();
        QMessageBox::critical(m_parent, "安装超时",
            "TAP 驱动安装超时（30秒）。\n\n"
            "请以管理员身份手动运行：\n"
            "  tapinstall.exe install OemVista.inf tap0901");
        return false;
    }

    int exitCode = p.exitCode();
    QString output = QString::fromLocal8Bit(p.readAllStandardOutput());
    QString errOut  = QString::fromLocal8Bit(p.readAllStandardError());
    qDebug() << "[EnvCheck] tapinstall 退出码:" << exitCode;
    qDebug() << "[EnvCheck] tapinstall 输出:" << output;
    if (!errOut.isEmpty())
        qDebug() << "[EnvCheck] tapinstall 错误:" << errOut;

    if (exitCode == 0) {
        QMessageBox::information(m_parent, "安装成功",
            "TAP 网卡驱动已成功安装！\n\n"
            "如后续 VPN 连接出现问题，请尝试重启计算机。");
        qDebug() << "[EnvCheck] TAP 驱动安装成功";
        return true;
    } else {
        QMessageBox::critical(m_parent, "安装失败",
            QString(
                "TAP 驱动安装失败（退出码：%1）。\n\n"
                "可能原因：\n"
                "  · 程序未以管理员身份运行\n"
                "  · 驱动签名验证失败（请检查 Windows 安全设置）\n\n"
                "请以管理员身份重新运行程序后重试。"
            ).arg(exitCode)
        );
        qDebug() << "[EnvCheck] TAP 驱动安装失败，退出码:" << exitCode;
        return false;
    }
}
