# M24 安装包与部署增强说明

## 1. 阶段目标

M24 在 portable 发布目录基础上补安装包脚本，让项目具备更接近企业交付的部署形态。

## 2. 新增文件

| 文件 | 用途 |
| --- | --- |
| `installer/upkun-hmi.iss` | Inno Setup 安装脚本模板 |
| `scripts/package_installer.ps1` | 先生成 portable 包，再尝试调用 ISCC 打安装包 |

## 3. 构建命令

```powershell
.\scripts\package_release.ps1
```

生成 portable 包。

```powershell
.\scripts\package_installer.ps1
```

如果本机安装了 Inno Setup，会继续生成安装程序；否则脚本保留 portable 包并提示安装 ISCC。

## 4. 部署边界

当前安装脚本是模板级实现，适合学习企业发布流程。后续可补：

- 安装前关闭正在运行的程序。
- 升级时保留 `data` 和 `config/app.ini`。
- 卸载时提示是否删除运行数据。
- 版本化发布说明。
