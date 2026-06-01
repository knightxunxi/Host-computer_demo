# 测试与部署说明

## 1. 当前 M8 产物

本阶段提供测试与部署的第一版工程化产物：

- `scripts/smoke_test.ps1`：构建程序、隐藏启动上位机、验证 Modbus TCP 模拟器、验证 SQLite 数据库创建。
- `scripts/regression_test.ps1`：构建程序并运行 CTest 回归测试，覆盖点位、数据库和协议。
- `scripts/package_release.ps1`：Release 构建，用 `windeployqt` 收集 Qt 运行库到 `dist/upkun-hmi`，并生成 `dist/upkun-hmi.zip`。
- `docs/09-test-and-deployment.md`：测试、打包和部署说明。

## 2. 烟测

执行：

```powershell
.\scripts\smoke_test.ps1
```

验证内容：

```text
1. CMake 配置和构建成功。
2. upkun-hmi.exe 可隐藏启动。
3. 独立 Modbus TCP 模拟器进程监听 127.0.0.1:1502。
4. 读取输入寄存器功能码 0x04 有响应。
5. 可以写 Coil 触发模拟故障和复位。
6. SQLite 数据库 data/app.sqlite3 被创建。
```

通过标志：

```text
Smoke test passed.
```

## 3. 回归测试

执行：

```powershell
.\scripts\regression_test.ps1
```

验证内容：

```text
1. CMake 配置和构建成功。
2. CTest 运行 upkun-regression。
3. 点位地址转换符合点位表。
4. SQLite 初始化能创建核心业务表。
5. 独立模拟器支持 Modbus TCP 写故障码、触发故障并读回报警码。
```

通过标志：

```text
100% tests passed
Regression tests passed.
```

## 4. Release 打包

执行：

```powershell
.\scripts\package_release.ps1
```

输出目录：

```text
dist/upkun-hmi
dist/upkun-hmi.zip
```

目录中应包含：

```text
upkun-hmi.exe
upkun-simulator.exe
Qt 运行库
platforms/
sqldrivers/
config/app.example.ini
```

## 5. 手工验收场景

建议每次阶段提交后手工验证：

1. 启动 `upkun-hmi.exe`。
2. 主监控页点击“启动”，确认产量和工位状态变化。
3. 模拟器页触发“缺盖”，确认报警码出现。
4. 报警记录页确认出现报警和操作日志。
5. 参数/配方页修改目标速度，保存并下发。
6. 趋势曲线页确认曲线持续刷新。
7. 趋势曲线页点击“导出趋势CSV”，确认 `data/trend-export.csv` 生成。
8. 通信诊断页确认请求数、响应数和通信质量持续刷新。
9. 关闭程序，重新启动，确认数据库不会丢失历史记录。

## 6. 部署注意事项

- 当前部署目标是 Windows 工控机或 Windows 开发机。
- 需要使用与 Qt 6.10.2 MinGW 匹配的运行库，打包脚本通过 `windeployqt` 收集。
- `data/` 和 `logs/` 是运行期数据目录，不提交 Git。
- `config/app.example.ini` 是示例配置，真实部署时可复制为 `config/app.ini` 后修改。
- 若目标机器端口 `1502` 被占用，模拟器启动会失败；后续会把端口配置做成界面设置项。

## 7. 当前限制

- 还没有 Inno Setup 安装包，当前交付形态是可运行目录。
- 当前 zip 是免安装 portable 包，不是 MSI/EXE 安装向导。
- 烟测是端到端基础检查，不替代完整 UI 手工测试。
- 已有轻量 CTest 回归测试；后续可继续引入 GoogleTest 扩展服务层单元测试。
