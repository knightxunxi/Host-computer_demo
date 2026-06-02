# M19 系统设置与连接管理说明

## 1. 阶段目标

M19 的目标是把设备连接参数从纯配置文件推进到界面可见、可保存、可重连的状态。这样后续切换真实设备、外部模拟器或不同协议时，不需要改代码。

## 2. 新增能力

- 新增“系统设置”页面。
- 支持配置 Modbus TCP 地址、端口。
- 支持配置 Modbus RTU 串口名、波特率、从站 ID。
- 支持配置轮询周期、超时和重连间隔。
- 支持配置数据库路径和日志目录。
- 支持 `auto_start_simulator`，用于控制上位机启动时是否自动拉起本机模拟器。
- 保存配置会写入 `config/app.ini`。

## 3. 连接模式

| 场景 | 设置方式 | 行为 |
| --- | --- | --- |
| 学习默认模式 | `modbus_tcp` + 自动启动模拟器 | 上位机启动 `upkun-simulator.exe` 并连接本机端口 |
| 外部模拟器 | `modbus_tcp` + 关闭自动启动 | 上位机只连接配置的 TCP 端点 |
| RTU 学习 | `modbus_rtu` | 使用 RTU 客户端骨架和诊断提示 |

## 4. 验证方式

```powershell
cmake --build build
.\scripts\regression_test.ps1
.\scripts\smoke_test.ps1
```
