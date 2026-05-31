# 配方、趋势曲线与 CSV 导出说明

## 1. 当前实现范围

本阶段完成 M7 的第一版能力：

```text
RecipePage
  -> RecipeRepository 保存默认配方
  -> ModbusTcpClient.writeRecipe 下发 Holding Registers

DeviceSnapshot
  -> TrendPage 实时刷新曲线
  -> TrendRepository 写入 trend_samples
  -> TrendRepository.exportCsv 导出 data/trend-export.csv
```

当前支持：

- 左侧新增“参数/配方”页面。
- 支持编辑目标速度、灌装量、灌装时间、旋盖扭矩、重量上下限、贴标模式、批次目标、模拟合格率。
- 支持保存配方到 SQLite `recipes` 表。
- 支持将配方下发到模拟 PLC 的 `40001-40010` Holding Registers。
- 左侧新增“趋势曲线”页面。
- 趋势曲线实时显示速度、灌装量、重量。
- 趋势采样写入 SQLite `trend_samples` 表。
- 支持导出趋势 CSV 到 `data/trend-export.csv`。

## 2. 数据库表

### `recipes`

主要字段：

```text
name
target_speed
fill_volume
fill_time
capping_torque
weight_min
weight_max
label_mode
batch_target_count
simulation_quality_rate
updated_at
```

### `trend_samples`

主要字段：

```text
sample_time
speed
fill_volume
weight
torque
temperature
pressure
```

## 3. 手工验证

运行程序：

```powershell
.\build\upkun-hmi.exe
```

配方验证：

1. 打开“参数/配方”页面。
2. 修改目标速度或灌装量。
3. 点击“保存配方”。
4. 点击“下发到PLC/模拟器”。
5. 打开“报警记录”页面，应看到保存/下发配方操作日志。

趋势验证：

1. 打开“主监控”页面。
2. 点击“启动”。
3. 打开“趋势曲线”页面，应看到曲线持续追加采样点。
4. 点击“导出趋势CSV”。
5. 检查 `data/trend-export.csv` 是否生成。

数据库验证：

```powershell
python - <<'PY'
import sqlite3
conn = sqlite3.connect(r'data/app.sqlite3')
cur = conn.cursor()
for table in ('recipes', 'trend_samples'):
    cur.execute(f'select count(*) from {table}')
    print(table, cur.fetchone()[0])
conn.close()
PY
```

## 4. 当前限制

- 目前只有一个默认配方，后续再扩展配方列表、新增、删除和切换。
- 趋势曲线只显示最近内存采样点，历史趋势查询后续再扩展。
- CSV 导出当前只覆盖趋势数据，生产记录和报警报表导出后续再统一做报表中心。
- M10 已新增批次管理第一版；趋势采样暂未写入批次号，后续报表中心再统一关联趋势、批次和报警数据。
