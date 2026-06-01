# M17 部署增强说明

## 1. 阶段目标

M17 的目标是让学习项目更接近企业交付形态。当前仍然是 portable 目录发布，不做安装向导和自动升级，但需要具备这些基础能力：

- 程序有明确版本号。
- 配置文件能覆盖默认连接参数。
- 数据和日志目录清晰。
- 发布包包含运行所需的两个程序和 Qt 运行库。
- 发布前有检查清单。

## 2. 版本号

CMake 项目版本调整为：

```text
0.17.0
```

构建时通过 `UPKUN_APP_VERSION` 编译宏传给：

- `upkun-hmi.exe`
- `upkun-simulator.exe`

HMI 窗口标题会显示版本号，便于测试截图和问题反馈时确认版本。

## 3. 配置读取

程序启动时按以下顺序读取配置：

1. 优先读取 `config/app.ini`。
2. 如果不存在，则读取 `config/app.example.ini`。
3. 如果两个文件都不存在，则使用代码默认值。

当前配置项包括：

| 配置项 | 用途 |
| --- | --- |
| `database/path` | SQLite 数据库路径 |
| `log/path` | 日志目录 |
| `device/host` | 模拟器或 PLC 地址 |
| `device/port` | 模拟器或 PLC 端口 |
| `device/poll_status_ms` | 状态轮询周期 |
| `device/poll_process_ms` | 过程数据轮询周期，后续扩展预留 |
| `device/timeout_ms` | 通信超时时间 |
| `device/reconnect_ms` | 自动重连间隔 |

## 4. 日志

启动时会初始化日志目录，并写入：

```text
logs/app-yyyyMMdd.log
```

当前日志记录应用启动、关闭和 Qt 日志消息。操作级审计仍保存在 SQLite 的 `operation_logs` 表中。

## 5. 打包目录

执行：

```powershell
.\scripts\package_release.ps1
```

发布目录包含：

```text
dist/upkun-hmi/
  upkun-hmi.exe
  upkun-simulator.exe
  config/app.example.ini
  data/
  logs/
  RELEASE-CHECKLIST.md
  platforms/
  sqldrivers/
  Qt*.dll
```

## 6. 发布检查

新增：

```text
docs/release-checklist.md
```

打包时会复制为：

```text
dist/upkun-hmi/RELEASE-CHECKLIST.md
```

发布前建议依次运行：

```powershell
.\scripts\regression_test.ps1
.\scripts\smoke_test.ps1
.\scripts\package_release.ps1
```

## 7. 后续进入 M18

M18 会在现有 Modbus TCP 基础上扩展一个真实设备协议方向。学习优先级建议选择 Modbus RTU，因为它和当前点位模型最接近，也方便用虚拟串口或协议文档继续学习。
