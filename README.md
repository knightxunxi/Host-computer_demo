# C++ Qt 工业上位机学习项目

本项目用于按企业轻量开发流程学习工业上位机开发。第一版业务场景选择“小型包装产线上位机系统”，重点覆盖需求分析、架构设计、Qt 桌面界面、工业通信、数据记录、报警、权限、测试与部署等完整链路。

## 项目定位

- 学习目标：通过一个完整项目理解工业上位机从需求到交付的开发流程。
- 业务场景：模拟一条小型包装产线，上位机负责设备监控、控制操作、报警处理、生产记录和配方管理。
- 当前阶段：M12 报警增强第一版已进入实现，支持报警筛选、详情和处理建议。
- 后续设备接入：没有真实 PLC 前，默认使用仓库内置虚拟包装线/PLC 模拟器，并通过协议模拟学习 TCP Socket、Modbus TCP、串口、Modbus RTU、OPC UA 和 Snap7/S7。

## 技术栈规划

| 方向 | 选型 |
| --- | --- |
| 语言 | C++20 |
| UI 框架 | Qt 6.10.2 Widgets |
| 构建系统 | CMake |
| 编译工具链 | MinGW 64-bit |
| 通信方向 | TCP Socket、Modbus TCP/RTU、串口、OPC UA、Snap7/S7 |
| 数据存储 | SQLite |
| 曲线与报表 | Qt Charts 或 QCustomPlot、CSV/Excel 导出 |
| 日志 | Qt 日志系统或 spdlog |
| 测试 | GoogleTest、手工联调测试 |
| 部署 | windeployqt、Inno Setup |

## 文档入口

- [需求分析](docs/01-requirements-analysis.md)
- [开发流程](docs/02-development-process.md)
- [项目路线图](docs/03-roadmap.md)
- [产线模型与典型用例](docs/04-line-model-and-use-cases.md)
- [总体设计](docs/05-overall-design.md)
- [Modbus TCP 模拟链路说明](docs/06-modbus-simulation.md)
- [报警与操作日志模块说明](docs/07-alarm-and-operation-log.md)
- [配方、趋势曲线与 CSV 导出说明](docs/08-recipe-trend-export.md)
- [测试与部署说明](docs/09-test-and-deployment.md)
- [M8 测试报告](docs/10-test-report.md)
- [M8 后续开发路线](docs/11-post-m8-roadmap.md)
- [M9 用户与权限说明](docs/12-user-permission.md)
- [M10 批次管理说明](docs/13-batch-management.md)
- [M11 配方增强说明](docs/14-recipe-management.md)
- [M12 报警增强说明](docs/15-alarm-management.md)

## 当前实现能力

- Qt Widgets 主窗口、主监控页和模拟器页。
- 内置 Modbus TCP 模拟器，默认监听 `127.0.0.1:1502`。
- Modbus TCP 客户端自动连接内置模拟器并轮询状态。
- 主监控页可发送启动、停止、复位、报警确认命令。
- 模拟器页可触发缺瓶、缺盖、缺标签报警并清除故障。
- 报警记录页可查看最近报警和最近操作日志。
- SQLite 自动创建 `data/app.sqlite3`，用于保存报警和操作日志。
- 参数/配方页可保存默认配方并下发到模拟 PLC。
- 趋势曲线页可显示速度、灌装量、重量，并导出 `data/trend-export.csv`。
- M9 开始补充用户表、中文显示名、快速切换用户、角色权限和真实操作审计。
- M10 新增批次管理页，可生成批次号、开始/结束批次、保存批次产量和良率。
- M11 新增多配方列表、版本递增、复制配方和配方下发记录。
- M12 新增报警状态/等级/工位/关键词筛选、报警详情和处理建议。

## 构建运行

本机 Qt 安装路径按 `D:\QT\6.10.2\mingw_64` 规划，编译器使用 Qt 自带的 `D:\QT\Tools\mingw1310_64`。不要混用其他 MinGW 版本，否则可能出现 Qt 链接错误。

```powershell
$env:Path='D:\QT\6.10.2\mingw_64\bin;D:\QT\Tools\mingw1310_64\bin;' + $env:Path
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=D:\QT\6.10.2\mingw_64 -DCMAKE_CXX_COMPILER=D:\QT\Tools\mingw1310_64\bin\g++.exe
cmake --build build
```

运行：

```powershell
.\build\upkun-hmi.exe
```

烟测：

```powershell
.\scripts\smoke_test.ps1
```

打包：

```powershell
.\scripts\package_release.ps1
```

## 阶段路线

1. 仓库初始化与需求分析。
2. 总体设计与 Qt/CMake 项目骨架。
3. 内置虚拟 PLC/包装线模拟器。
4. 协议模拟学习、上位机通信、状态监控与基础控制。
5. 报警、数据库、趋势曲线和配方。
6. 用户权限、批次管理、配方增强和报警闭环。
7. 测试、打包、部署、模拟器拆分和协议扩展。

## Git 约定

- 默认分支：`main`
- 提交信息建议使用简化 Conventional Commits：
  - `docs:` 文档变更
  - `feat:` 功能新增
  - `fix:` 缺陷修复
  - `test:` 测试相关
  - `chore:` 工程配置或杂项
