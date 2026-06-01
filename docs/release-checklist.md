# Upkun HMI 发布检查清单

## 1. 构建产物

- `upkun-hmi.exe` 存在。
- `upkun-simulator.exe` 存在。
- `platforms/qwindows.dll` 存在。
- `sqldrivers/qsqlite.dll` 存在。
- `config/app.example.ini` 存在。
- `data/` 目录存在。
- `logs/` 目录存在。

## 2. 启动检查

- 启动 `upkun-hmi.exe` 后窗口标题显示版本号。
- 模拟器自动启动并监听配置中的地址和端口。
- 通信诊断页显示已连接。
- 主监控页启动后产量、工位和趋势数据正常刷新。

## 3. 功能检查

- 基础控制：启动、停止、复位、报警确认可操作。
- 模拟故障：缺盖、重量不合格、下料满料等故障能触发报警。
- 报警页面：报警可筛选、显示详情和处理建议。
- 配方页面：可保存、复制、下发配方，并记录下发日志。
- 批次页面：可开始、结束批次，并记录产量和良率。
- 趋势页面：可导出 CSV。

## 4. 数据与日志

- `data/app.sqlite3` 能自动创建。
- `logs/app-yyyyMMdd.log` 能自动创建。
- 操作日志中能看到启动、控制、报警、配方、批次等操作。

## 5. 回归验证

发布前建议在源码目录执行：

```powershell
.\scripts\regression_test.ps1
.\scripts\smoke_test.ps1
.\scripts\package_release.ps1
```

三个脚本都通过后，再交付 `dist/upkun-hmi` 或 `dist/upkun-hmi.zip`。
