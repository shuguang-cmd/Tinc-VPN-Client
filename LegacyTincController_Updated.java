package com.ruoyi.web.controller.system;

import com.alibaba.fastjson2.JSONObject;
import com.ruoyi.TincNetworkMange.domain.TincNetworkMange;
import com.ruoyi.TincNetworkMange.service.ITincNetworkMangeService;
import com.ruoyi.common.annotation.Anonymous;
import com.ruoyi.common.core.domain.AjaxResult;
import com.ruoyi.common.utils.StringUtils;
import com.ruoyi.common.utils.TincConfigUtils;
import com.ruoyi.manger.domain.MangeServer;
import com.ruoyi.manger.service.IMangeServerService;
import com.ruoyi.node_manage.domain.TincNodeMange;
import com.ruoyi.node_manage.service.ITincNodeMangeService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;
import javax.servlet.http.HttpServletResponse;
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.Map;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

@Anonymous
@RestController
@RequestMapping("/XVntQFJCjc.php")
public class LegacyTincController {

    private static final Logger log = LoggerFactory.getLogger(LegacyTincController.class);

    @Autowired
    private IMangeServerService mangeServerService;

    @Autowired
    private ITincNetworkMangeService networkMangeService;

    @Autowired
    private ITincNodeMangeService nodeMangeService;

    @PostMapping("/myadmin/node/api")
    public JSONObject login(@RequestBody Map<String, String> params) {
        log.info("========== 登录接口被调用 ==========");
        log.info("请求参数: {}", params);

        String username = params.get("sid");
        String password = params.get("password");

        log.info("用户名: {}, 密码长度: {}", username, password != null ? password.length() : 0);

        JSONObject result = new JSONObject();

        try {
            if (StringUtils.isEmpty(username) || StringUtils.isEmpty(password)) {
                log.warn("用户名或密码为空，登录失败");
                result.put("status", 0);
                result.put("msg", "用户名或密码不能为空");
                return result;
            }

            TincNodeMange queryNode = new TincNodeMange();
            queryNode.setNodeName(username);
            List<TincNodeMange> nodeList = nodeMangeService.selectTincNodeMangeList(queryNode);

            if (nodeList == null || nodeList.isEmpty()) {
                log.warn("用户不存在: {}", username);
                result.put("status", 0);
                result.put("msg", "用户不存在");
                return result;
            }

            TincNodeMange node = nodeList.get(0);

            if (!password.equals(node.getPasswrod())) {
                log.warn("密码错误: {}", username);
                result.put("status", 0);
                result.put("msg", "密码错误");
                return result;
            }

            TincNetworkMange queryNetwork = new TincNetworkMange();
            queryNetwork.setNetworkName(node.getNetworkName());
            List<TincNetworkMange> networkList = networkMangeService.selectTincNetworkMangeList(queryNetwork);

            if (networkList == null || networkList.isEmpty()) {
                log.warn("网络不存在: {}", node.getNetworkName());
                result.put("status", 0);
                result.put("msg", "网络不存在");
                return result;
            }

            TincNetworkMange network = networkList.get(0);

            result.put("sid", username);
            result.put("status", 1);
            result.put("token", "sun-token-" + System.currentTimeMillis());
            result.put("net_name", network.getNetworkName());
            result.put("msg", "登录成功");

            log.info("登录成功，用户: {}, 网络: {}", username, network.getNetworkName());
        } catch (Exception e) {
            log.error("登录接口异常", e);
            result.put("status", 0);
            result.put("msg", "系统错误: " + e.getMessage());
        }

        return result;
    }

    @PostMapping("/coreplugs/coreplugs/api")
    public void downloadConfig(@RequestBody Map<String, String> params, HttpServletResponse response) throws IOException {
        log.info("========== 下载配置接口被调用 ==========");
        log.info("请求参数: {}", params);

        String nodeName = params.get("sid");
        String token = params.get("token");

        log.info("节点名称: {}, Token: {}", nodeName, token);

        try {
            TincNodeMange queryNode = new TincNodeMange();
            queryNode.setNodeName(nodeName);
            List<TincNodeMange> nodeList = nodeMangeService.selectTincNodeMangeList(queryNode);

            if (nodeList == null || nodeList.isEmpty()) {
                log.error("节点不存在: {}", nodeName);
                throw new IOException("节点不存在");
            }

            TincNodeMange node = nodeList.get(0);

            TincNetworkMange queryNetwork = new TincNetworkMange();
            queryNetwork.setNetworkName(node.getNetworkName());
            List<TincNetworkMange> networkList = networkMangeService.selectTincNetworkMangeList(queryNetwork);

            if (networkList == null || networkList.isEmpty()) {
                log.error("网络不存在: {}", node.getNetworkName());
                throw new IOException("网络不存在");
            }

            TincNetworkMange network = networkList.get(0);

            MangeServer queryServer = new MangeServer();
            queryServer.setServerName(network.getServerName());
            List<MangeServer> serverList = mangeServerService.selectMangeServerList(queryServer);

            String serverIp = network.getServerName();
            if (serverList != null && !serverList.isEmpty()) {
                MangeServer server = serverList.get(0);
                serverIp = server.getServerIp();
                log.info("从mange_server表获取服务器IP: {}", serverIp);
            } else {
                log.warn("未在mange_server表找到服务器，使用network.server_name: {}", serverIp);
            }

            response.setContentType("application/zip");
            response.setHeader("Content-Disposition", "attachment; filename=\"config.zip\"");

            try (ZipOutputStream zos = new ZipOutputStream(response.getOutputStream())) {

                String tincConf = "Name = " + nodeName + "\n" +
                        "Interface = tinc0\n" +
                        "Mode = router\n" +
                        "ConnectTo = server_master\n";
                addToZip(zos, network.getNetworkName() + "/tinc.conf", tincConf);
                log.info("已添加 {}/tinc.conf", network.getNetworkName());

                String serverHostContent = TincConfigUtils.readHostFile(network.getNetworkName(), "server_master");
                if (serverHostContent == null) {
                    TincNodeMange queryServerNode = new TincNodeMange();
                    queryServerNode.setNodeName("server_master");
                    queryServerNode.setNetworkName(network.getNetworkName());
                    List<TincNodeMange> serverNodeList = nodeMangeService.selectTincNodeMangeList(queryServerNode);

                    if (serverNodeList != null && !serverNodeList.isEmpty()) {
                        TincNodeMange serverNode = serverNodeList.get(0);
                        serverHostContent = "Address = " + serverIp + "\n" +
                                "Subnet = " + serverNode.getnetworkIp() + "/32\n" +
                                "-----BEGIN PUBLIC KEY-----\n" +
                                serverNode.getPasswrod() + "\n" +
                                "-----END PUBLIC KEY-----\n";
                    } else {
                        serverHostContent = "Address = " + serverIp + "\n" +
                                "Subnet = 10.0.10.1/32\n" +
                                "-----BEGIN PUBLIC KEY-----\n" +
                                "服务器公钥内容\n" +
                                "-----END PUBLIC KEY-----\n";
                        log.warn("未找到服务器节点，使用默认配置");
                    }
                }
                addToZip(zos, network.getNetworkName() + "/hosts/server_master", serverHostContent);
                log.info("已添加 {}/hosts/server_master", network.getNetworkName());

                String tincUp = "#!/bin/sh\n" +
                        "ifconfig $INTERFACE " + node.getnetworkIp() + " netmask 255.255.255.0\n";
                addToZip(zos, network.getNetworkName() + "/tinc-up", tincUp);
                log.info("已添加 {}/tinc-up", network.getNetworkName());

                zos.flush();
                log.info("配置包生成完成");
            }
        } catch (Exception e) {
            log.error("生成配置包异常", e);
            throw e;
        }
    }

    @PostMapping("/coreplugs/Clientinterface/exchangeFile")
    public void uploadKey(@RequestBody Map<String, String> params, HttpServletResponse response) throws IOException {
        log.info("========== 公钥上传接口被调用 ==========");
        log.info("请求参数: {}", params);

        String nodeName = params.get("sid");
        String publicKeyContent = params.get("content");
        String action = params.get("action");

        log.info("节点名称: {}, 操作类型: {}, 公钥长度: {}", nodeName, action,
                 publicKeyContent != null ? publicKeyContent.length() : 0);

        try {
            TincNodeMange queryNode = new TincNodeMange();
            queryNode.setNodeName(nodeName);
            List<TincNodeMange> nodeList = nodeMangeService.selectTincNodeMangeList(queryNode);

            if (nodeList == null || nodeList.isEmpty()) {
                log.error("节点不存在: {}", nodeName);
                throw new IOException("节点不存在");
            }

            TincNodeMange node = nodeList.get(0);

            TincNetworkMange queryNetwork = new TincNetworkMange();
            queryNetwork.setNetworkName(node.getNetworkName());
            List<TincNetworkMange> networkList = networkMangeService.selectTincNetworkMangeList(queryNetwork);

            if (networkList == null || networkList.isEmpty()) {
                log.error("网络不存在: {}", node.getNetworkName());
                throw new IOException("网络不存在");
            }

            TincNetworkMange network = networkList.get(0);

            MangeServer queryServer = new MangeServer();
            queryServer.setServerName(network.getServerName());
            List<MangeServer> serverList = mangeServerService.selectMangeServerList(queryServer);

            String serverIp = network.getServerName();
            if (serverList != null && !serverList.isEmpty()) {
                MangeServer server = serverList.get(0);
                serverIp = server.getServerIp();
                log.info("从mange_server表获取服务器IP: {}", serverIp);
            } else {
                log.warn("未在mange_server表找到服务器，使用network.server_name: {}", serverIp);
            }

            String finalHostContent = "Subnet = " + node.getnetworkIp() + "/32\n\n" + publicKeyContent;
            TincConfigUtils.createHostFile(network.getNetworkName(), nodeName, node.getnetworkIp() + "/32", publicKeyContent);

            node.setStatus("已配置");
            nodeMangeService.updateTincNodeMange(node);
            log.info("节点状态已更新: {}", nodeName);

            String mainHostContent = TincConfigUtils.readHostFile(network.getNetworkName(), "server_master");
            if (mainHostContent == null) {
                TincNodeMange queryServerNode = new TincNodeMange();
                queryServerNode.setNodeName("server_master");
                queryServerNode.setNetworkName(network.getNetworkName());
                List<TincNodeMange> serverNodeList = nodeMangeService.selectTincNodeMangeList(queryServerNode);

                if (serverNodeList != null && !serverNodeList.isEmpty()) {
                    TincNodeMange serverNode = serverNodeList.get(0);
                    mainHostContent = "Address = " + serverIp + "\n" +
                            "Subnet = " + serverNode.getnetworkIp() + "/32\n" +
                            "-----BEGIN PUBLIC KEY-----\n" +
                            serverNode.getPasswrod() + "\n" +
                            "-----END PUBLIC KEY-----\n";
                } else {
                    mainHostContent = "Address = " + serverIp + "\n" +
                            "Subnet = 10.0.10.1/32\n" +
                            "-----BEGIN PUBLIC KEY-----\n" +
                            "服务器公钥内容\n" +
                            "-----END PUBLIC KEY-----\n";
                    log.warn("未找到服务器节点，使用默认配置");
                }
            }

            response.setContentType("text/plain");
            response.setCharacterEncoding("UTF-8");
            response.getWriter().write(mainHostContent);
            log.info("服务器主机配置已返回");
        } catch (Exception e) {
            log.error("公钥上传接口异常", e);
            throw e;
        }
    }

    @PostMapping("/promin/Api/editadd_info")
    public JSONObject editAddInfo(@RequestBody Map<String, String> params) {
        log.info("========== 编辑接口被调用 ==========");
        log.info("请求参数: {}", params);

        String type = params.get("type");
        String result = params.get("result");
        String ids = params.get("ids");
        String details = params.get("details");

        log.info("操作类型: {}, 结果: {}, ID: {}, 详情: {}", type, result, ids, details);

        try {
            if ("add".equals(type) && StringUtils.isNotEmpty(ids)) {
                TincNodeMange queryNode = new TincNodeMange();
                queryNode.setId(Long.parseLong(ids));
                List<TincNodeMange> nodeList = nodeMangeService.selectTincNodeMangeList(queryNode);

                if (nodeList != null && !nodeList.isEmpty()) {
                    TincNodeMange node = nodeList.get(0);

                    if ("success".equals(result)) {
                        node.setStatus("配置成功");
                        node.setNodeStatus("在线");
                    } else {
                        node.setStatus("配置失败");
                        node.setNodeStatus("离线");
                    }

                    nodeMangeService.updateTincNodeMange(node);
                    log.info("节点状态已更新: {}, 状态: {}", node.getNodeName(), node.getStatus());
                }
            }

            JSONObject resp = new JSONObject();
            resp.put("status", "success");
            log.info("编辑接口返回: {}", resp);
            return resp;
        } catch (Exception e) {
            log.error("编辑接口异常", e);
            JSONObject resp = new JSONObject();
            resp.put("status", "error");
            resp.put("msg", "系统错误: " + e.getMessage());
            return resp;
        }
    }

    private void addToZip(ZipOutputStream zos, String fileName, String content) throws IOException {
        ZipEntry entry = new ZipEntry(fileName);
        zos.putNextEntry(entry);
        zos.write(content.getBytes(StandardCharsets.UTF_8));
        zos.closeEntry();
    }
}
