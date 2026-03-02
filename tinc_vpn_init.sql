-- ==========================================
-- Tinc VPN 数据库初始化脚本
-- ==========================================
-- 说明：此脚本用于初始化Tinc VPN系统所需的数据库表和初始数据
-- 使用方法：在MySQL数据库中执行此脚本
-- ==========================================

-- 设置字符集
SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ==========================================
-- 1. Tinc内网集群管理表
-- ==========================================
DROP TABLE IF EXISTS `tinc_network_mange`;
CREATE TABLE `tinc_network_mange` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'ids',
  `root_name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '' COMMENT '用户',
  `server_name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '' COMMENT '接入服务器',
  `network_name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '' COMMENT '内网名称',
  `create_time` datetime NOT NULL DEFAULT current_timestamp() COMMENT '创建时间',
  `port` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '' COMMENT '端口',
  `segment` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '' COMMENT '网段',
  `nodes` int(11) NOT NULL DEFAULT 0 COMMENT '节点数量',
  `network_status` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '' COMMENT '内网状态',
  `explanation` varchar(255) DEFAULT NULL COMMENT '备注',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_network_name` (`network_name`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Tinc内网集群管理';

-- ==========================================
-- 2. Tinc节点集群管理表
-- ==========================================
DROP TABLE IF EXISTS `tinc_node_mange`;
CREATE TABLE `tinc_node_mange` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'ids',
  `use_name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '' COMMENT '用户',
  `table_ID` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '' COMMENT '设备ID',
  `server_name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '' COMMENT '接入服务器',
  `network_name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '' COMMENT '所属内网',
  `password` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '' COMMENT '密码',
  `node_name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '' COMMENT '节点名称',
  `network_ip` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '' COMMENT '内网ip',
  `explantion` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '' COMMENT '备注',
  `create_time` datetime DEFAULT current_timestamp() COMMENT '创建时间',
  `node_status` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '' COMMENT '节点状态',
  `status` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '' COMMENT '配置状态',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_node_name_network` (`node_name`, `network_name`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Tinc节点集群管理';

-- ==========================================
-- 3. 插入初始数据
-- ==========================================

-- 插入默认网络配置
INSERT INTO `tinc_network_mange` (`root_name`, `server_name`, `network_name`, `port`, `segment`, `nodes`, `network_status`, `explanation`) VALUES
('admin', '122.152.232.241', 'MyFirstNet', '655', '10.0.10.0/24', 0, 'active', '默认内网配置');

-- 插入服务器节点配置
INSERT INTO `tinc_node_mange` (`use_name`, `table_ID`, `server_name`, `network_name`, `password`, `node_name`, `network_ip`, `explantion`, `node_status`, `status`) VALUES
('admin', 'server_001', '122.152.232.241', 'MyFirstNet', 'server_password', 'server_master', '10.0.10.1', '服务器节点', 'online', '已配置');

-- 插入测试客户端节点配置
INSERT INTO `tinc_node_mange` (`use_name`, `table_ID`, `server_name`, `network_name`, `password`, `node_name`, `network_ip`, `explantion`, `node_status`, `status`) VALUES
('admin', 'client_001', '122.152.232.241', 'MyFirstNet', '123456', 'admin', '10.0.10.100', '测试客户端节点', 'offline', '未配置');

-- ==========================================
-- 4. 查询验证
-- ==========================================

-- 查询网络配置
SELECT * FROM tinc_network_mange WHERE network_name = 'MyFirstNet';

-- 查询节点配置
SELECT * FROM tinc_node_mange WHERE network_name = 'MyFirstNet';

-- ==========================================
-- 5. 使用说明
-- ==========================================
-- 
-- 1. 登录接口测试：
--    POST /XVntQFJCjc.php/myadmin/node/api
--    Body: {"sid": "admin", "password": "123456"}
--    预期返回：{"sid": "admin", "status": 1, "token": "sun-token-xxx", "net_name": "MyFirstNet", "msg": "登录成功"}
--
-- 2. 下载配置接口测试：
--    POST /XVntQFJCjc.php/coreplugs/coreplugs/api
--    Body: {"sid": "admin", "token": "sun-token-xxx"}
--    预期返回：config.zip文件
--
-- 3. 公钥上传接口测试：
--    POST /XVntQFJCjc.php/coreplugs/Clientinterface/exchangeFile
--    Body: {"sid": "admin", "content": "公钥内容", "action": "add"}
--    预期返回：服务器配置文件内容
--
-- 4. 编辑接口测试：
--    POST /XVntQFJCjc.php/promin/Api/editadd_info
--    Body: {"type": "add", "result": "success", "ids": "1", "details": "节点admin已完成接入"}
--    预期返回：{"status": "success"}
--
-- ==========================================
-- 6. 注意事项
-- ==========================================
-- 
-- 1. 密码字段（password）在tinc_node_mange表中用于存储客户端登录密码
-- 2. 节点状态（node_status）可选值：online、offline
-- 3. 配置状态（status）可选值：未配置、已配置、配置成功、配置失败
-- 4. network_ip字段必须符合CIDR格式（如：10.0.10.100）
-- 5. 每个网络中只能有一个服务器节点（node_name = 'server_master'）
-- 6. 客户端节点的node_name必须唯一
--
-- ==========================================

SET FOREIGN_KEY_CHECKS = 1;
