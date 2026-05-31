# M10 批次管理说明

## 1. 本阶段目标

M10 的目标是让生产数据可以按批次追溯。当前实现是学习版批次闭环：

- 有批次表。
- 有批次管理页面。
- 可以生成批次号。
- 可以开始和结束批次。
- 可以记录批次计划数量、产量、良品、不良、良率、操作员和配方。
- 批次开始/结束会写操作日志。

## 2. 批次流程

推荐手工操作顺序：

1. 打开“参数/配方”，确认当前配方和批次目标。
2. 打开“批次管理”，生成或输入批次号。
3. 点击“开始批次”。
4. 打开“主监控”，点击“启动”，让模拟产线运行一段时间。
5. 点击“停止”。
6. 回到“批次管理”，点击“结束批次”。
7. 查看最近批次记录。

当前实现要求产线停止后再结束批次。这是为了贴近现场习惯：先停机或完成当前生产，再归档批次数据。

## 3. 数据库表

新增 `batches` 表：

| 字段 | 说明 |
| --- | --- |
| `batch_no` | 批次号，唯一 |
| `recipe_name` | 开始批次时的配方名称 |
| `operator_user_id` | 操作员用户 ID |
| `operator_login_name` | 操作员登录名快照 |
| `operator_display_name` | 操作员中文显示名快照 |
| `target_count` | 计划数量 |
| `start_total_count` | 开始批次时的总产量基线 |
| `start_good_count` | 开始批次时的良品基线 |
| `start_bad_count` | 开始批次时的不良品基线 |
| `total_count` | 批次数量 |
| `good_count` | 批次良品 |
| `bad_count` | 批次不良 |
| `status` | `Running`、`Completed`、`Aborted` |
| `started_at` | 开始时间 |
| `ended_at` | 结束时间 |

开始时保存生产计数基线，结束时用当前计数减去基线得到本批次良品和不良品。

## 4. 通信点位

批次管理复用已有 Modbus 线圈：

| 点位 | 名称 | 说明 |
| --- | --- | --- |
| `00007` | `CMD_BATCH_START` | 开始批次，模拟器清零当前批次数量 |
| `00008` | `CMD_BATCH_END` | 结束批次，模拟器清零当前批次数量 |
| `30014` | `IR_BATCH_COUNT` | 当前批次数量 |
| `40008` | `HR_BATCH_TARGET_COUNT` | 配方中的批次目标数量 |

## 5. 权限规则

操作员、工程师和管理员都可以开始/结束批次。系统会记录当前登录用户，方便追溯。

## 6. 手工验证

运行程序：

```powershell
.\build\upkun-hmi.exe
```

验证步骤：

1. 确认顶部显示默认用户“张三（操作员）”。
2. 打开“批次管理”，点击“生成批次号”。
3. 点击“开始批次”，应看到当前批次进入“进行中”。
4. 打开“主监控”，点击“启动”，等待批次数量增长。
5. 点击“停止”。
6. 回到“批次管理”，点击“结束批次”。
7. 最近批次表应出现该批次，状态为“已完成”。
8. 打开“报警记录”，最近操作应包含“开始批次”和“结束批次”。

数据库验证：

```powershell
@'
import sqlite3
conn = sqlite3.connect('data/app.sqlite3')
cur = conn.cursor()
for row in cur.execute('select batch_no, recipe_name, operator_display_name, target_count, total_count, good_count, bad_count, status from batches order by id desc limit 5'):
    print(row)
conn.close()
'@ | python -
```

## 7. 当前限制

- 当前只有一个默认配方，M11 会扩展多配方和版本。
- 趋势数据还没有写入批次号，后续报表中心再统一关联。
- 批次结束目前只支持正常完成，后续可增加中止、返工、异常结束。
- 批次页面没有导出功能，后续报表模块再处理。
