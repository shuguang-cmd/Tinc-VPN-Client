#ifndef ENVCHECK_H
#define ENVCHECK_H

#include <QObject>
#include <QString>
#include <QWidget>

/**
 * @brief 启动前环境检查类
 *
 * 在登录界面显示前执行以下检查：
 * 1. 检查 tincd.exe 是否存在于 Tinc/ 目录
 * 2. 检查 TAP 虚拟网卡驱动（tap0901）是否已安装
 *    - 若未安装，提示用户并自动调用 tapinstall.exe 安装
 */
class EnvCheck : public QObject
{
    Q_OBJECT
public:
    explicit EnvCheck(QWidget *parent = nullptr);

    /**
     * @brief 执行所有环境检查
     * @return true 表示环境就绪，可以继续启动；false 表示致命错误，应退出
     */
    bool runChecks();

    /** 获取 Tinc 安装目录路径（检查完成后可用） */
    QString tincDir() const { return m_tincDir; }

private:
    /** 检查 tincd.exe 是否存在 */
    bool checkTincd();

    /** 检查 TAP 网卡驱动是否已安装 */
    bool checkTapDriver();

    /** 调用 tapinstall.exe 安装 TAP 驱动 */
    bool installTapDriver();

    /** 解析 Tinc 和 TAP 目录路径 */
    void resolvePaths();

    QString m_tincDir;  ///< Tinc 安装目录（含 tincd.exe）
    QString m_tapDir;   ///< TAP 驱动目录（含 tapinstall.exe）
    QWidget *m_parent;
};

#endif // ENVCHECK_H
