# Dots and Boxes AI（点格棋）

基于**并行蒙特卡洛搜索 + 尾局双叉（double-cross）策略**的点格棋 AI，带 Qt6 图形界面，支持人机 / 机机对战、自定义棋盘大小与对弈双方。

![demo](demo.png)

## 核心创新

### 尾局双叉（Double-Cross）策略

点格棋的胜负往往由尾局决定。传统 AI 在尾局采用贪心策略，遇到链就全部拿完，却将「开链主动权」拱手送给对手。本项目引入 Berlekamp 提出的双叉策略：

- **拿链时双叉**——取链时只拿「链长 − 2」格，故意留下最后 2 格让出，逼对手替自己开下一链，从而保住「控制权」；
- **安全棋优先**——开局 / 中盘只下不送格的「安全棋」；
- **被迫开链时开最短链**——当所有空边都会送格时，选择让对手拿得最少的边（Berlekamp 标准走法）。

这一策略被植入底层 rollout，整条 `ImproveGreedyRobot → SimulationRobot → MonteCarloRobot → ParallelSearchRobot` 全栈自动受益：蒙特卡洛的胜率估计更准确，最终落子整体更强。

### 技术亮点

- **编译期模板化棋盘**——C++23 编译期参数化棋盘尺寸，运行时零开销；
- **Zobrist 哈希 + LRU 缓存**——快速判定重复局面，缓存搜索结果，避免重复计算；
- **Intel TBB 并行搜索**——多线程并行蒙特卡洛 rollout，充分利用多核性能；
- **O(1) 落子更新**——增量维护棋盘状态，每一步落子即时生效；
- **Qt6 交互式图形界面**——支持鼠标点击画线、实时分数统计、人机 / 机机对战。

## 功能特性

- Qt6 交互式图形界面
- 多级 AI 对手：
  - `GreedyRobot`（贪心）
  - `ImproveGreedyRobot`（改进贪心 + 尾局双叉）
  - `SimulationRobot`（前向模拟）
  - `MonteCarloRobot`（蒙特卡洛）
  - `ParallelSearchRobot`（TBB 并行搜索，默认）
- 可配置棋盘大小（1–16）
- 分数统计与显示
- 棋盘可视化

## 环境要求

- 支持 C++23 的编译器
- Qt6（Core、Gui、Widgets）
- CMake 3.16 及以上
- 依赖通过 git submodule 拉取（`deps/tbb`、`deps/lru`），无需单独安装 TBB

## 构建

### 前置：初始化子模块

```bash
git submodule update --init --recursive
```

### macOS / Linux

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel --target Dots_and_Boxes
./build/Dots_and_Boxes
```

也可直接运行 `bash build.sh`（默认 Debug）。

### Windows

```bat
git submodule update --init --recursive
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel --target Dots_and_Boxes
```

或直接运行 `build.bat`。打包成独立可执行程序时，用 Qt 自带的 `windeployqt` 收集运行库：

```bat
windeployqt build\Release\Dots_and_Boxes.exe
```

### 预编译可执行程序

本仓库提供 GitHub Actions 自动构建（Windows `.exe`、macOS `.app`），在打 `v*` 标签时触发，产物上传到对应 Release。详见 [.github/workflows/release.yml](.github/workflows/release.yml)。

## 使用

### 基本操作

1. 启动程序；
2. 用鼠标点击边线来「画线」；
3. 框满四条边即得分，得分后继续走；
4. 最终占格多者获胜。

### 命令行参数

| 选项 | 短 | 长 | 说明 | 默认 |
| --- | --- | --- | --- | --- |
| 棋盘大小 | `-s` | `--boardsize` | 1–16 | 6 |
| 玩家 1 | `-p1` | `--player1` | 玩家类型 | robot |
| 玩家 2 | `-p2` | `--player2` | 玩家类型 | robot |

玩家类型取值：`human`、`GreedyRobot`、`ImproveGreedyRobot`、`SimulationRobot`、`MonteCarloRobot`、`ParallelSearchRobot`（`robot` 等价于 `ParallelSearchRobot`）。

```bash
# 默认设置
./Dots_and_Boxes

# 10x10 棋盘
./Dots_and_Boxes --boardsize 10

# 玩家 1 是人类，玩家 2 用蒙特卡洛
./Dots_and_Boxes --player1 human --player2 MonteCarloRobot
```

## 项目结构

```
Dots-and-Boxes/
├── .github/workflows/     # CI：编译 + 发布打包
├── deps/                  # 子模块：tbb、lru
├── src/
│   ├── robot/             # AI 实现（Greedy/ImproveGreedy/Simulation/MonteCarlo/ParallelSearch）
│   ├── frontend/          # Qt6 界面（MainWindow、各 Canvas）
│   ├── Board.h            # 编译期模板棋盘 + Zobrist 哈希
│   └── Model.h            # Box / Edge / Dot 数据模型
├── main.cpp               # 程序入口
├── CMakeLists.txt
├── build.sh / build.bat
└── demo.png
```

## 许可协议

[MIT 协议](LICENSE)
