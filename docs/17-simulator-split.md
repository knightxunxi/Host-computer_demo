# M14 模拟器拆分说明

## 1. 阶段目标

M14 的目标是建立更接近真实设备联调的边界：

- 上位机进程负责界面、业务、报警、权限、数据和日志。
- 模拟器进程负责虚拟 PLC 点位、产线状态机和 Modbus TCP 响应。
- 两者之间只通过 `127.0.0.1:1502` 上的 Modbus TCP 交互。

这样后续接真实 PLC 时，上位机侧的业务逻辑不需要知道“对面是模拟器还是真设备”。

## 2. 可执行文件

构建后会生成两个程序：

| 程序 | 作用 |
| --- | --- |
| `upkun-hmi.exe` | 上位机主程序 |
| `upkun-simulator.exe` | 包装产线/PLC 模拟器 |

上位机启动时会用 `QProcess` 自动拉起 `upkun-simulator.exe`，然后用 `ModbusTcpClient` 连接模拟器。模拟器页面的“启动模拟器/停止模拟器”控制的是这个独立进程。

## 3. 协议边界变化

M14 前，模拟器页触发故障时，上位机会直接调用进程内对象：

```text
SimulatorPage -> SimulatedModbusServer::triggerAlarm()
```

M14 后，故障注入改为协议读写：

```text
SimulatorPage
  -> ModbusTcpClient::injectFault(alarmCode)
  -> 写 40011 模拟故障码
  -> 写 00010 模拟故障线圈
  -> upkun-simulator.exe 内部触发对应报警
```

清除故障也改为发送复位命令，而不是直接调用 `clearAlarm()`。

## 4. 点位补充

M14 新增一个保持寄存器：

| 地址 | 名称 | 用途 |
| --- | --- | --- |
| `40011` | `SimFaultCode` | 上位机写入要注入的模拟报警码 |

原有线圈 `00010` 保留为模拟故障触发命令。模拟器收到该线圈后读取 `40011`，再触发对应报警。

## 5. 验证方式

1. 构建项目：

```powershell
cmake --build build
```

2. 确认两个程序存在：

```powershell
Test-Path .\build\upkun-hmi.exe
Test-Path .\build\upkun-simulator.exe
```

3. 运行烟测：

```powershell
.\scripts\smoke_test.ps1
```

4. 手工验证：

- 启动 `upkun-hmi.exe`。
- 打开模拟器页，确认监听状态为 `127.0.0.1:1502`。
- 点击“触发缺盖”或“下料满料”，主监控和报警页应出现对应报警。
- 点击“清除模拟故障”，再执行复位和启动。

## 6. 当前限制

- 模拟器仍由上位机自动启动，暂未提供独立配置文件。
- 模拟器仍监听本机地址，暂未支持远程设备部署。
- 故障注入使用学习版专用点位，真实 PLC 项目应由 PLC 程序或调试工具提供等价测试信号。
- M15 已在此协议边界上补充通信超时、重连统计、通信质量标识和诊断页。
