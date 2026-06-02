# M18 协议扩展说明

## 1. 阶段目标

M18 在既有 Modbus TCP 基础上扩展一个新的真实设备协议方向。本阶段选择 Modbus RTU，原因是：

- 它和当前点位表、线圈、寄存器模型最接近。
- 可以继续复用启动、停止、复位、配方下发等业务语义。
- 后续有虚拟串口或真实串口设备时，可以在同一接口下补真实收发。

当前机器未确认可用 QtSerialPort，因此 M18 先完成 RTU 帧编码、CRC 校验和 `IDeviceClient` 适配骨架，不强行加入串口依赖。

## 2. 新增模块

| 文件 | 作用 |
| --- | --- |
| `src/device/ModbusRtuCodec.*` | RTU 帧编码、CRC16 计算和 CRC 校验 |
| `src/device/ModbusRtuClient.*` | 实现 `IDeviceClient` 的 RTU 客户端骨架 |

`ModbusRtuCodec` 当前支持：

- `0x03` 读保持寄存器。
- `0x04` 读输入寄存器。
- `0x05` 写单线圈。
- `0x06` 写单保持寄存器。

## 3. 配置项

`config/app.example.ini` 新增 RTU 相关配置：

```ini
[device]
mode=modbus_tcp
host=127.0.0.1
port=1502
serial_port=COM1
baud_rate=9600
slave_id=1
```

当前默认仍是 `modbus_tcp`。如果改为：

```ini
mode=modbus_rtu
```

上位机会创建 `ModbusRtuClient`，但由于当前构建未启用串口传输，界面会显示通信错误。这个状态是预期的学习版边界。

## 4. 接口关系

TCP 和 RTU 都遵循同一个业务接口：

```text
IDeviceClient
  <- ModbusTcpClient
  <- ModbusRtuClient
```

UI 和业务层仍然只调用：

- `connectToDevice`
- `disconnectFromDevice`
- `sendCommand`
- `writeRecipe`

协议差异封装在设备客户端内部。

## 5. 回归测试

M18 在 `tests/regression_tests.cpp` 中增加 RTU 编码检查：

- 生成 `01 03 00 00 00 0A C5 CD`。
- 验证 CRC 可以通过。
- 截断帧不能通过 CRC。

运行：

```powershell
.\scripts\regression_test.ps1
```

## 6. 后续扩展

如果后续安装 QtSerialPort，可以继续补：

- 串口打开、关闭和参数设置。
- RTU 请求队列、超时和重试。
- RTU 响应解析。
- 虚拟串口联调脚本。
- 与当前通信诊断页共享超时、错误和重连统计。
