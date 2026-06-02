# M22 RTU 串口条件接入说明

## 1. 阶段目标

M22 在 M18 的 RTU 帧编码基础上，补充 QtSerialPort 的条件检测。当前本机 Qt 目录未检测到 QtSerialPort，因此默认构建仍保持 RTU 编码学习和诊断提示。

## 2. 当前实现

- CMake 使用 `find_package(Qt6 COMPONENTS SerialPort QUIET)` 检测串口模块。
- 检测到模块时定义 `UPKUN_HAS_QT_SERIALPORT=1`。
- 未检测到模块时，RTU 客户端会明确提示“未检测到 QtSerialPort，仅提供 Modbus RTU 帧编码学习”。

## 3. 后续真实串口实现点

如果安装 QtSerialPort，后续可继续补：

- 串口打开、关闭和参数设置。
- RTU 请求队列。
- 响应帧缓存、CRC 校验和功能码解析。
- 超时、重试和诊断统计。
- 虚拟串口联调脚本。

## 4. 验证

当前可验证项是 RTU CRC/帧编码回归测试和配置切换后的诊断提示。
