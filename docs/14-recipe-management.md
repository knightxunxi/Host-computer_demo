# M11 配方增强说明

## 1. 本阶段目标

M11 的目标是让配方管理从“单个默认配方”升级为更接近现场使用的“多配方 + 版本 + 下发追溯”。

当前实现范围：

- 多配方列表。
- 配方加载。
- 新建配方。
- 复制为新配方。
- 保存同名配方时版本号递增。
- 配方下发记录。
- 下发记录包含用户、角色、配方版本和目标设备。

## 2. 数据库变化

### `recipes`

M11 在原有配方字段基础上增加：

| 字段 | 说明 |
| --- | --- |
| `version` | 配方版本，保存同名配方时递增 |
| `created_at` | 创建时间 |
| `updated_by` | 最近更新人 |
| `last_applied_at` | 最近下发时间 |

旧数据库会在程序启动时自动补齐这些字段，不需要手工删库。

### `recipe_apply_logs`

新增下发记录表：

| 字段 | 说明 |
| --- | --- |
| `recipe_id` | 下发时的配方 ID |
| `recipe_name` | 下发时的配方名称快照 |
| `recipe_version` | 下发时的版本 |
| `display_name` | 操作用户中文名 |
| `role` | 操作用户角色 |
| `target` | 下发目标，例如 PLC/模拟器 |
| `result` | 下发结果 |
| `message` | 附加说明 |
| `created_at` | 下发时间 |

## 3. 界面行为

“参数/配方”页面现在包含三块：

1. 左侧配方编辑区：编辑当前配方参数。
2. 右上配方列表：双击某一行加载该配方。
3. 右下最近下发记录：查看配方下发历史。

按钮行为：

| 按钮 | 行为 |
| --- | --- |
| 新建配方 | 在编辑区生成一个未保存的新配方 |
| 复制为新配方 | 基于当前已保存配方生成副本 |
| 保存配方 | 保存当前配方；同名保存会递增版本 |
| 下发到PLC/模拟器 | 先保存，再写入 Modbus Holding Registers，并记录下发日志 |
| 刷新列表 | 重新读取配方列表和下发记录 |

## 4. 权限规则

保存、复制和下发配方仍要求工程师或管理员。默认登录的操作员不能修改工艺参数，需要点击顶部“切换用户”，使用工程师账号：

```text
eng001 / 123456
```

## 5. 手工验证

运行程序：

```powershell
.\build\upkun-hmi.exe
```

验证步骤：

1. 点击顶部“切换用户”，选择“李工（eng001，工程师）”，输入 `123456`。
2. 打开“参数/配方”。
3. 双击配方列表中的“默认配方”，确认编辑区加载。
4. 点击“复制为新配方”，应生成副本。
5. 修改目标速度或灌装量。
6. 点击“保存配方”，配方列表应出现新配方。
7. 再次点击“保存配方”，该配方版本应递增。
8. 点击“下发到PLC/模拟器”，最近下发记录应出现该配方和版本。
9. 打开“报警记录”，最近操作应包含保存配方和下发配方。

数据库验证：

```powershell
@'
import sqlite3
conn = sqlite3.connect('data/app.sqlite3')
cur = conn.cursor()
print('recipes')
for row in cur.execute('select name, version, updated_by, last_applied_at from recipes order by id'):
    print(row)
print('apply logs')
for row in cur.execute('select recipe_name, recipe_version, display_name, result from recipe_apply_logs order by id desc limit 5'):
    print(row)
conn.close()
'@ | python -
```

## 6. 当前限制

- 还没有删除、禁用、导入、导出配方。
- 还没有配方审批和电子签名。
- 版本目前保存在 `recipes.version` 单字段，没有独立版本历史表。
- 下发记录保存的是下发动作，不保存完整参数快照；后续可扩展 `recipe_versions` 或 `recipe_snapshots`。
