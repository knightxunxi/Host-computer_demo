# C++ Qt 工业上位机学习项目

本项目用于按企业轻量开发流程学习工业上位机开发。第一版业务场景选择“小型包装产线上位机系统”，重点覆盖需求分析、架构设计、Qt 桌面界面、工业通信、数据记录、报警、权限、测试与部署等完整链路。

## 项目定位

- 学习目标：通过一个完整项目理解工业上位机从需求到交付的开发流程。
- 业务场景：模拟一条小型包装产线，上位机负责设备监控、控制操作、报警处理、生产记录和配方管理。
- 当前阶段：首阶段只初始化仓库和需求文档，不创建 CMake/Qt 代码骨架。
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

## 阶段路线

1. 仓库初始化与需求分析。
2. 总体设计与 Qt/CMake 项目骨架。
3. 内置虚拟 PLC/包装线模拟器。
4. 协议模拟学习、上位机通信、状态监控与基础控制。
5. 报警、数据库、趋势曲线、配方和权限。
6. 测试、打包、部署和项目复盘。

## Git 约定

- 默认分支：`main`
- 提交信息建议使用简化 Conventional Commits：
  - `docs:` 文档变更
  - `feat:` 功能新增
  - `fix:` 缺陷修复
  - `test:` 测试相关
  - `chore:` 工程配置或杂项
