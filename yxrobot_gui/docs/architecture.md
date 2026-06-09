# 上位机架构说明

本文档梳理 `yxrobot_gui` 当前上位机的主要模块、数据流和地图图层架构，便于后续维护、扩展和调试。

## 总体分层

工程整体可以分为五层：

1. **应用入口层**
   - `src/app/main.cpp`
   - 创建 `QApplication`，注册跨线程 Qt 元类型，启动 `MainWindow`。

2. **主窗口层**
   - `src/mainwindow/mainwindow.*`
   - 负责加载 UI、初始化通信通道、把地图视图状态同步到状态栏。

3. **地图面板层**
   - `src/mainwindow/map_panel/view`
   - `MapGraphicsView` 是地图交互入口，负责鼠标拖拽、缩放、右键图层菜单、区域编辑按钮、PNC 任务按钮和 P2P 箭头预览。
   - `MapOverlayWidget` 是悬浮按钮条，向 View 发出按钮点击信号。

4. **地图图层层**
   - `src/mainwindow/map_panel/core`
   - `MapLayerBase` 是所有图层的共同基类。
   - `MapDisplayFactory` 负责创建默认图层。
   - `MapLayerRegistry` 保存图层元信息并统一控制显隐。
   - `MapLayerRuntime` 把图层加入 `QGraphicsScene`，并把通信信号绑定到具体图层。

5. **通信适配层**
   - `src/channel`
   - `VirtualChannel` 定义上位机内部统一通信接口。
   - `ChannelManager` 根据构建选项动态加载 ROS1/ROS2 插件。
   - `ros1/qnode.*` 和 `ros2/rclcomm.*` 负责把 ROS 消息转换为上位机内部结构，或把上位机命令编码后发布到 ROS 话题。

## 核心数据类型

公共数据类型放在 `src/common` 下：

- `Point`：二维点模板别名，主要用于地图、路径、激光点云。
- `OccupancyMap`：栅格地图数据，包含尺寸、分辨率、原点和 Eigen 矩阵。
- `RobotPose`：机器人位姿。
- `LaserScan`：激光点集合。
- `Path`：路径点集合。
- `MapCoordinateTransformer`：负责世界坐标和场景坐标互转。
- `NavigationZoneCollection` / `NavigationPncTask`：定义在 `common.h` 中的不依赖 Qt/ROS 的导航命令结构。

## 地图图层架构

当前默认图层由 `MapDisplayFactory::createDefaultDisplays()` 创建，顺序和职责如下：

| 图层 | id | z 值 | 职责 |
| --- | --- | --- | --- |
| 占据栅格地图 | `map.occMap` | 0 | 绘制全局地图 |
| 全局代价地图 | `map.globalCostMap` | 10 | 绘制代价地图 |
| 网格 | `grid.grid` | 12 | 根据缩放绘制辅助网格 |
| 机器人位姿 | `localization.robot` | 15 | 绘制机器人位置和朝向 |
| 激光 | `scan.laser` | 20 | 绘制激光点云 |
| 全局路径 | `plan.globalPath` | 25 | 绘制规划路径 |
| 编辑区域 | `map.editableZones` | 30 | 绘制禁区、虚拟墙、障碍物、家具等可编辑区域 |

图层生命周期：

1. `MapGraphicsView` 构造 `QGraphicsScene`。
2. `MapLayerRuntime::initializeDefaultLayers()` 调用工厂创建默认图层。
3. 每个图层被加入 scene，并注册到 `MapLayerRegistry`。
4. 右键菜单读取 registry，按 group 分类展示图层显隐开关。
5. `MapLayerRuntime::bindChannel()` 把通信信号连接到图层更新函数。

## 地图交互流程

### 地图刷新

1. ROS 通道收到地图消息。
2. 通道转换为 `OccupancyMap`。
3. `VirtualChannel::emitUpdateMap` 发到 UI 线程。
4. `MapGraphicsView::updateMap()` 更新坐标转换器、scene 范围、网格范围和编辑层地图转换器。
5. 各图层按自己的 `updateMap()` 更新缓存并触发重绘。

### 鼠标和缩放

- 左键/中键默认用于平移地图。
- 滚轮围绕鼠标位置缩放，并限制最小/最大缩放。
- 鼠标移动时持续发出 scene/world 坐标，用于状态栏显示。

### P2P 点到点任务

当前 P2P 模式采用类似 RViz 的交互：

1. 点击“点到点”按钮进入 P2P 选择模式。
2. 鼠标按下时记录目标位置的世界坐标。
3. 鼠标拖动时在 scene 中显示蓝色方向箭头。
4. 鼠标释放时记录释放点，用按下点到释放点的方向计算 yaw。
5. 发送 `NavigationPncTask`，其中 `goal = {x, y, yaw}`。
6. 通信层将 P2P 任务转换为 `geometry_msgs/PoseStamped` 风格的位姿消息；ROS1 发布到 `goal_pose`，ROS2 当前发布到 `move_base_simple/goal`。
7. 若按下和释放几乎重合，则不发送任务，只提示继续拖拽方向。

## 编辑区域架构

编辑区域由三部分组成：

1. **数据模型：`EditableZoneModel`**
   - 保存所有编辑区域。
   - 支持新增/更新、删除、清空。
   - 按业务类型拆分为 planning zones 和 blocked areas。
   - 可转换为不依赖 Qt/ROS 的 `NavigationZoneCollection`。

2. **图层：`EditableZoneLayerItem`**
   - 持有 `EditableZoneModel`。
   - 负责绘制已有区域、绘制中的线段、选中区域控制点。
   - 管理区域绘制状态、选中状态和提交信号。

3. **视图控制：`MapGraphicsView`**
   - 负责按钮事件和鼠标事件。
   - 把 view 坐标转换为 world 坐标后交给编辑图层。
   - 保证 P2P 模式和区域绘制模式互斥。

区域提交流程：

1. 用户完成、删除或清空区域。
2. `EditableZoneLayerItem` 发出 `zoneCommitRequested(zones)`。
3. `MapLayerRuntime` 把信号连接到 `VirtualChannel`。
4. ROS2 通道先发布 `visualization_msgs/msg/Marker::DELETEALL` 清理旧显示。
5. ROS2 通道把每个区域转换为 `visualization_msgs/msg/Marker` 并发布到 `yxrobot_gui/navigation_zone_markers`。
6. 多边形区域使用闭合 `LINE_STRIP`，虚拟墙使用两点 `LINE_STRIP`，每种区域通过 `marker.ns` 和颜色区分。

## 通信插件架构

`ChannelManager` 根据 `YX_CHANNEL_OPTION` 选择通信后端：

- `none`：不加载通信插件。
- `ros1`：加载 `libchannel_ros1.so`。
- `ros2`：加载 `libchannel_ros2.so`。

插件通过两个 C 接口暴露实例：

- `GetChannelInstance()`
- `DestroyChannelInstance(VirtualChannel*)`

主程序只依赖 `VirtualChannel`，不直接依赖 ROS1/ROS2 的具体类型。这样 UI 和地图图层可以保持相对稳定，通信适配可以独立替换。

## 数据流总结

### ROS 到 UI

```text
ROS topic
  -> ROS channel callback
  -> common 数据结构
  -> VirtualChannel Qt signal
  -> MapGraphicsView / LayerItem
  -> QGraphicsScene repaint
```

### UI 到 ROS

```text
按钮/鼠标交互
  -> MapGraphicsView
  -> EditableZoneModel / PncTaskModel
  -> common navigation command
  -> VirtualChannel
  -> ROS channel adapter
  -> ROS publisher / service / action
```

## 当前设计重点

- 图层绘制和通道适配已经基本解耦。
- 编辑区域和 PNC 命令已经从 JSON 字符串升级为结构化自定义格式。
- ROS2/ROS1 编辑区域统一使用 `Marker` 发布，不再沿用旧文本编码协议。
- ROS1 点到点任务发布到 `goal_pose`，ROS2 点到点任务发布到当前配置的 `move_base_simple/goal`。
- Qt 类型主要留在 UI/model 边界，通道发送接口使用标准库结构。
- `MapGraphicsView` 当前承担较多交互协调职责，后续若继续增加编辑能力，可考虑抽出独立的交互 controller。
