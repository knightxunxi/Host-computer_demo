# 测试与部署说明

## 1. 当前 M8 产物

本阶段提供测试与部署的第一版工程化产物：

- `scripts/smoke_test.ps1`：构建程序、隐藏启动上位机、验证 Modbus TCP 模拟器、验证 SQLite 数据库创建。
- `scripts/package_release.ps1`：Release 构建，并用 `windeployqt` 收集 Qt 运行库到 `dist/upkun-hmi`。
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
3. 内置 Modbus TCP 模拟器监听 127.0.0.1:1502。
4. 读取输入寄存器功能码 0x04 有响应。
5. 可以写 Coil 触发模拟故障和复位。
6. SQLite 数据库 data/app.sqlite3 被创建。
```

通过标志：

```text
Smoke test passed.
```

## 3. Release 打包

执行：

```powershell
.\scripts\package_release.ps1
```

输出目录：

```text
dist/upkun-hmi
```

目录中应包含：

```text
upkun-hmi.exe
Qt 运行库
platforms/
sqldrivers/
config/app.example.ini
```

## 4. 手工验收场景

建议每次阶段提交后手工验证：

1. 启动 `upkun-hmi.exe`。
2. 主监控页点击“启动”，确认产量和工位状态变化。
3. 模拟器页触发“缺盖”，确认报警码出现。
4. 报警记录页确认出现报警和操作日志。
5. 参数/配方页修改目标速度，保存并下发。
6. 趋势曲线页确认曲线持续刷新。
7. 趋势曲线页点击“导出趋势CSV”，确认 `data/trend-export.csv` 生成。
8. 关闭程序，重新启动，确认数据库不会丢失历史记录。

## 5. 部署注意事项

- 当前部署目标是 Windows 工控机或 Windows 开发机。
- 需要使用与 Qt 6.10.2 MinGW 匹配的运行库，打包脚本通过 `windeployqt` 收集。
- `data/` 和 `logs/` 是运行期数据目录，不提交 Git。
- `config/app.example.ini` 是示例配置，真实部署时可复制为 `config/app.ini` 后修改。
- 若目标机器端口 `1502` 被占用，模拟器启动会失败；后续会把端口配置做成界面设置项。

## 6. 当前限制

- 还没有 Inno Setup 安装包，当前交付形态是可运行目录。
- 烟测是端到端基础检查，不替代完整 UI 手工测试。
- 自动化测试尚未覆盖业务服务单元测试；后续可引入 GoogleTest。
