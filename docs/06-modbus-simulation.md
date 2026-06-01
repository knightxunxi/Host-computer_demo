# Modbus TCP 模拟链路说明

## 1. 当前实现范围

本阶段实现了最小可运行的内置 Modbus TCP 模拟链路：

```text
MainWindow
  -> SimulatedModbusServer 监听 127.0.0.1:1502
  -> LineSimulator 维护虚拟 PLC 点位和产线状态
  -> ModbusTcpClient 连接本机模拟器并轮询点位
  -> MonitorPage 刷新产量、状态、报警和活跃工位
```

当前支持：

- 应用启动时自动启动内置模拟器。
- Modbus TCP Server 监听 `127.0.0.1:1502`。
- Modbus TCP Client 周期读取离散输入和输入寄存器。
- 主监控页按钮写入启动、停止、复位、报警确认命令。
- 模拟器页可触发急停、气压不足、缺瓶、堵瓶、缺盖、重量不合格、缺标签、剔除失败、下料满料等故障，也可清除模拟报警。
- 模拟器会输出速度扰动、灌装量波动、重量波动、扭矩波动和随机不良结果。

## 2. 支持的 Modbus 功能码

| 功能码 | 名称 | 当前用途 |
| --- | --- | --- |
| `0x01` | Read Coils | 读取命令位状态 |
| `0x02` | Read Discrete Inputs | 读取安全、传感器和就绪状态 |
| `0x03` | Read Holding Registers | 读取配方/参数寄存器 |
| `0x04` | Read Input Registers | 读取系统状态、产量和过程数据 |
| `0x05` | Write Single Coil | 写启动、停止、复位等命令 |
| `0x06` | Write Single Register | 写单个参数 |
| `0x10` | Write Multiple Registers | 批量下发配方参数 |

## 3. 关键代码入口

| 文件 | 作用 |
| --- | --- |
| `src/simulator/LineSimulator.*` | 虚拟产线状态机和点位数据 |
| `src/simulator/SimulatedModbusServer.*` | Modbus TCP 服务端 |
| `src/device/ModbusTcpClient.*` | Modbus TCP 客户端 |
| `src/device/ModbusPointMap.h` | 点位地址常量和地址转换 |
| `src/app/MainWindow.*` | 启动模拟器、连接客户端、桥接 UI |
| `src/ui/pages/MonitorPage.*` | 显示快照并发出控制命令 |
| `src/ui/pages/SimulatorPage.*` | 触发和清除模拟故障 |

## 4. 手工验证命令

构建：

```powershell
$env:Path='D:\QT\6.10.2\mingw_64\bin;D:\QT\Tools\mingw1310_64\bin;' + $env:Path
cmake --build build
```

运行：

```powershell
.\build\upkun-hmi.exe
```

界面验证：

1. 打开主监控页。
2. 点击“启动”，系统状态应变为运行中，活跃工位轮流变化，产量逐步增长。
3. 打开模拟器页，点击“触发缺盖”，当前报警应变为 `5001`。
4. 点击“清除模拟故障”，再回主监控点击“复位”和“启动”。

协议验证示例：读取输入寄存器 `30001-30005`。

```powershell
$client = [System.Net.Sockets.TcpClient]::new()
$client.Connect('127.0.0.1', 1502)
$stream = $client.GetStream()
[byte[]]$request = 0,1,0,0,0,6,1,4,0,0,0,5
$stream.Write($request, 0, $request.Length)
$buffer = New-Object byte[] 256
$read = $stream.Read($buffer, 0, $buffer.Length)
($buffer[0..($read-1)] | ForEach-Object { $_.ToString('X2') }) -join ' '
$client.Close()
```

正常响应应包含：

```text
00 01 00 00 ... 01 04 ...
```

其中 `0x04` 表示输入寄存器读取响应。

## 5. M13 增强点位

M13 后，模拟器的离散输入不再只用数字偏移描述，而是按点位语义命名：

| 类型 | 示例点位 | 用途 |
| --- | --- | --- |
| 公共联锁 | `EstopOk`、`SafetyDoorOk`、`AirPressureOk`、`PlcReady` | 判断产线是否具备启动条件 |
| 物料/到位 | `FeedingMaterialReady`、`BottleAtFilling`、`BottleAtCapping`、`BottleAtOutfeed` | 模拟瓶子在不同工位的到位状态 |
| 执行机构 | `ConveyorRunning`、`FillingValveOk`、`CapFeederReady`、`RejectCylinderHome` | 模拟输送、灌装、理盖、剔除等机构状态 |
| 质量检测 | `ScaleReady`、`WeightOk`、`WeightNg`、`RejectDetected` | 模拟重量检测和不良品剔除 |
| 下料状态 | `OutfeedReady`、`OutfeedJam` | 模拟下料通畅或满料堵塞 |

输入寄存器 `30015`、`30021-30025` 会随模拟器节拍变化：

| 地址 | 含义 |
| --- | --- |
| `30015` | 当前速度，基于目标速度增加轻微扰动 |
| `30021` | 实际灌装量，基于配方灌装量波动 |
| `30022` | 实际重量，按重量上下限和质量率生成 |
| `30023` | 实际旋盖扭矩，基于配方扭矩波动 |
| `30024` | 模拟温度 |
| `30025` | 模拟气压 |

## 6. 当前限制

- 模拟器和客户端目前运行在同一桌面进程内，后续再拆分到工作线程。
- 当前只实现学习用最小 Modbus TCP 子集，不包含完整异常恢复、超时队列和协议边界测试。
- 报警、配方、用户、批次已经有学习版闭环，但仍未接真实 PLC。
- 速度和质量波动用于上位机联调学习，不等同于真实包装机械的物理仿真。
- 真实 PLC、串口、Modbus RTU、OPC UA、Snap7/S7 仍是后续扩展。
