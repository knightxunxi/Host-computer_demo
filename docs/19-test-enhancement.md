# M16 测试增强说明

## 1. 阶段目标

M16 的目标是把项目从“能手工验证”推进到“能一键跑基础回归”。本阶段先不引入 GoogleTest，而是使用 Qt/C++ 编写轻量测试程序，并通过 CTest 统一执行。

选择轻量测试的原因：

- 当前项目重点是学习上位机全流程，不希望测试框架安装成为额外阻塞。
- CMake/CTest 已经足够覆盖第一批点位、数据库和协议回归。
- 后续需要更细的服务层单元测试时，再平滑迁移到 GoogleTest。

## 2. 新增测试入口

新增文件：

| 文件 | 作用 |
| --- | --- |
| `tests/regression_tests.cpp` | 轻量回归测试主程序 |
| `scripts/regression_test.ps1` | 构建并运行 CTest 的脚本 |

CMake 新增：

- `enable_testing()`
- `UpkunRegressionTests` 测试目标
- `upkun-regression` CTest 用例

## 3. 当前覆盖范围

### 3.1 点位映射测试

验证典型 Modbus 地址转换：

- 启动线圈 `00001` 的报文偏移为 `0`。
- 模拟故障线圈 `00010` 的报文偏移为 `9`。
- 当前报警输入寄存器 `30003` 的报文偏移为 `2`。
- 模拟故障码保持寄存器 `40011` 的报文偏移为 `10`。

这类测试能防止后续修改点位表时悄悄破坏协议地址。

### 3.2 数据库初始化测试

测试使用临时 SQLite 数据库，验证启动初始化能创建核心表：

- `alarms`
- `users`
- `recipes`
- `batches`
- `trend_samples`

这类测试能防止数据库迁移脚本改动后遗漏核心业务表。

### 3.3 模拟器协议测试

测试直接启动 `SimulatedModbusServer`，通过 TCP 客户端发送 Modbus 请求：

1. 写保持寄存器 `40011 = 9001`。
2. 写线圈 `00010 = true` 触发模拟故障。
3. 读输入寄存器 `30003`，确认当前报警码为 `9001`。

这验证了 M14/M15 之后“上位机通过协议注入故障”的核心边界。

## 4. 运行方式

执行：

```powershell
.\scripts\regression_test.ps1
```

脚本会执行：

```powershell
cmake -S . -B build ...
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

成功时会看到：

```text
Regression tests passed.
100% tests passed
```

## 5. 与烟测的区别

| 测试 | 关注点 |
| --- | --- |
| `smoke_test.ps1` | 真实启动 HMI，验证程序、模拟器端口和数据库是否能跑起来 |
| `regression_test.ps1` | 不启动完整 UI，直接验证点位、数据库和协议核心逻辑 |

两者都保留。阶段提交前建议都跑。

## 6. 后续进入 M17

M16 完成后，M17 已继续补充部署增强：

- 版本号显示。
- 配置文件读取。
- 日志目录规范。
- 发布包检查清单。
- 更接近企业交付的 portable 包说明。
