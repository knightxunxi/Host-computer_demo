# 报警与操作日志模块说明

## 1. 当前实现范围

本阶段实现了报警和操作日志的可验证闭环，并在 M12 扩展为可筛选、可查看详情和处理建议的报警页：

```text
DeviceSnapshot
  -> AlarmService 识别报警码变化
  -> AlarmRepository 写入 alarms
  -> OperationLogRepository 写入 operation_logs
  -> AlarmPage 显示报警筛选、报警详情、处理建议和最近操作
```

当前支持：

- 应用启动时初始化 SQLite 数据库 `data/app.sqlite3`。
- 自动创建 `alarms` 和 `operation_logs` 表。
- 轮询快照中出现新报警码时，写入报警记录。
- 报警码恢复为 `0` 时，更新报警恢复状态。
- 报警确认命令成功后，写入确认人和确认时间。
- 启动/停止模拟器、控制命令、模拟故障触发和报警变化会写操作日志。
- 左侧新增“报警记录”页面，展示报警记录、报警详情和最近操作。
- 支持按状态、等级、工位和关键词筛选报警。
- 支持选择报警后查看确认信息、恢复/关闭时间和处理建议。

## 2. 报警生命周期

| 状态 | 含义 |
| --- | --- |
| `ActiveUnacked` | 报警正在发生，未确认 |
| `ActiveAcked` | 报警正在发生，已确认 |
| `ClearedUnacked` | 报警条件已恢复，未确认 |
| `Closed` | 报警已恢复且已确认 |

当前模拟器故障恢复后，如果未点击“报警确认”，报警会停留在 `ClearedUnacked`。这是正常行为，表示现场问题已经消失，但还缺少人的确认。

## 3. 数据库表

### `alarms`

主要字段：

```text
alarm_code
alarm_name
station
level
state
triggered_at
acked_at
acked_by
cleared_at
closed_at
```

### `operation_logs`

主要字段：

```text
login_name
display_name
role
action
target
result
message
created_at
```

M9 后用户操作会记录当前登录用户；自动报警触发/恢复这类无人操作事件仍记录为系统日志。

## 4. 关键代码入口

| 文件 | 作用 |
| --- | --- |
| `src/storage/DatabaseManager.*` | 打开 SQLite 并初始化表结构 |
| `src/storage/AlarmRepository.*` | 写入和查询报警记录 |
| `src/storage/OperationLogRepository.*` | 写入和查询操作日志 |
| `src/services/AlarmService.*` | 根据快照维护报警生命周期 |
| `src/ui/pages/AlarmPage.*` | 显示最近报警和最近操作 |
| `src/app/MainWindow.*` | 桥接快照、命令反馈、模拟器操作和日志写入 |

## 5. 手工验证

运行程序：

```powershell
.\build\upkun-hmi.exe
```

界面验证：

1. 打开“模拟器”页面。
2. 点击“触发缺盖”。
3. 打开“报警记录”页面，应看到 `5001 缺盖`。
4. 使用状态、等级或工位筛选，应能筛出对应报警。
5. 选中该报警，应看到处理建议。
6. 回到“模拟器”页面，点击“清除模拟故障”或在主监控点击“复位”。
7. 回到“报警记录”页面刷新，报警状态应变为 `ClearedUnacked` 或 `Closed`。

数据库验证：

```powershell
python - <<'PY'
import sqlite3
conn = sqlite3.connect(r'data/app.sqlite3')
cur = conn.cursor()
for table in ('alarms', 'operation_logs'):
    cur.execute(f'select count(*) from {table}')
    print(table, cur.fetchone()[0])
conn.close()
PY
```

## 6. 当前限制

- 报警记录页已支持状态、等级、工位和关键词筛选；时间范围筛选后续再做。
- 操作日志已接入真实用户；自动事件仍使用系统用户。
- 数据库访问仍在 UI 进程内同步执行，数据量变大后再迁移到数据库工作线程。
- 报警确认只处理当前报警，批量确认和历史报警补确认后续再扩展。
