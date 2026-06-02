# 配方、趋势曲线与 CSV 导出说明

## 1. 当前实现范围

本阶段完成 M7 的第一版能力，并在 M11 扩展为多配方和下发追溯：

```text
RecipePage
  -> RecipeRepository 保存/复制/加载多配方
  -> ModbusTcpClient.writeRecipe 下发 Holding Registers
  -> RecipeRepository 记录 recipe_apply_logs

DeviceSnapshot
  -> TrendPage 实时刷新曲线
  -> TrendRepository 写入 trend_samples
  -> TrendRepository.exportCsv 导出 data/trend-export.csv
```

当前支持：

- 左侧新增“参数/配方”页面。
- 支持编辑目标速度、灌装量、灌装时间、旋盖扭矩、重量上下限、贴标模式、批次目标、模拟合格率。
- 支持保存配方到 SQLite `recipes` 表，保存同名配方时版本号自动递增。
- 支持配方列表、加载配方、复制为新配方。
- 支持将配方下发到模拟 PLC 的 `40001-40010` Holding Registers。
- 支持保存配方下发记录到 `recipe_apply_logs` 表。
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
version
created_at
updated_at
updated_by
last_applied_at
```

### `recipe_apply_logs`

主要字段：

```text
recipe_id
recipe_name
recipe_version
display_name
role
target
result
created_at
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
4. 点击“复制为新配方”，应生成一个新配方名。
5. 点击“保存配方”，配方列表应出现新配方；再次保存同名配方，版本号应递增。
6. 点击“下发到PLC/模拟器”。
7. 配方页下方“最近下发记录”应出现对应记录。
8. 打开“报警记录”页面，应看到保存/下发配方操作日志。

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
for table in ('recipes', 'recipe_apply_logs', 'trend_samples'):
    cur.execute(f'select count(*) from {table}')
    print(table, cur.fetchone()[0])
conn.close()
PY
```

## 4. 当前限制

- M11 已支持多配方、复制、版本递增和下发记录；删除、禁用、导入导出后续再扩展。
- 趋势曲线只显示最近内存采样点，历史趋势查询后续再扩展。
- CSV 导出当前包含趋势导出；M21 已新增报表中心，能汇总批次、报警和趋势数据并导出报表 CSV。
- M10 已新增批次管理第一版；趋势采样暂未写入批次号，后续可在报表中心继续关联趋势、批次和报警数据。
