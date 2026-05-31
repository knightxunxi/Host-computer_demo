# M8 测试报告

## 1. 测试环境

| 项目 | 内容 |
| --- | --- |
| 日期 | 2026-05-31 |
| 操作系统 | Windows |
| Qt | 6.10.2 MinGW |
| 编译器 | `D:\QT\Tools\mingw1310_64\bin\g++.exe` |
| 构建工具 | CMake + MinGW Makefiles |

## 2. 自动烟测

执行命令：

```powershell
.\scripts\smoke_test.ps1
```

结果：

```text
[100%] Built target UpkunHmi
Smoke test passed.
```

覆盖项：

| 检查项 | 结果 |
| --- | --- |
| CMake 配置 | 通过 |
| Debug 构建 | 通过 |
| 程序隐藏启动 | 通过 |
| 内置 Modbus TCP 模拟器监听 `127.0.0.1:1502` | 通过 |
| 输入寄存器读取功能码 `0x04` 响应 | 通过 |
| 写 Coil 触发模拟故障 | 通过 |
| 写 Coil 复位 | 通过 |
| SQLite 数据库创建 | 通过 |

## 3. Release 打包

执行命令：

```powershell
.\scripts\package_release.ps1
```

结果：

```text
[100%] Built target UpkunHmi
Package created: D:\C1\upkun\dist\upkun-hmi
Zip created: D:\C1\upkun\dist\upkun-hmi.zip
```

`windeployqt` 提示：

```text
Skipping plugin qopensslbackend.dll
Warning: Cannot find any version of the dxcompiler.dll and dxil.dll.
```

判定：当前应用不使用 OpenSSL 网络能力，也不依赖 Qt Quick 3D/D3D shader 编译路径；上述提示不阻塞当前 Widgets + SQLite + 本机 TCP 功能。

## 4. 打包产物烟测

执行对象：

```text
dist/upkun-hmi/upkun-hmi.exe
```

验证方式：

```text
隐藏启动打包目录中的 exe
连接 127.0.0.1:1502
读取输入寄存器 30001-30005
```

结果：

```text
packaged_bytes=19
```

判定：打包目录中的程序可启动，内置模拟器可响应 Modbus TCP 请求。

## 5. 当前结论

M8 首版通过：

- 测试脚本可重复执行。
- Release 目录可生成。
- Portable zip 包可生成。
- 打包目录中的程序可运行并响应核心协议烟测。

## 6. 已知限制

- 还没有 Inno Setup/MSI 安装向导。
- 自动化测试仍以端到端烟测为主，尚未引入 GoogleTest 单元测试。
- UI 手工测试需要在真实桌面环境中继续执行。
