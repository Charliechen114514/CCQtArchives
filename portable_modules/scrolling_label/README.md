# Scrolling Label

​	这个控件希望做到的是去生成一个简单的对于过长文本进行滚动播放效果的Label，当然，他需要保证文字长度是大于当前控件的显示长度（geometry.width）才是可行的。不然的话，可能就会伸展开来，失去播放的效果。

## 接口说明

#### 一、类名：`ScrollingLabel`

该类继承自 `QWidget`，属于一个自定义控件。通过内部计时器和偏移量实现对文本的动态滚动。

##### 类的核心功能：

- 设置文本内容；
- 判断是否需要滚动（依据控件宽度与文本长度）；
- 启动定时器以持续偏移绘制起点，从而达成滚动效果；
- 支持窗口大小改变时自动调整滚动逻辑。

#### 二、静态常量定义

- `ANIMATION_INTERVAL = 100`：定义了滚动的时间间隔，单位是毫秒。每隔 100ms 调用一次滚动函数，形成较平滑的动画效果。
- `STEP_SHORT = 2`：每次滚动的步长，单位为像素。即每次滚动时文本向左偏移的距离。

#### 三、公开接口

- `ScrollingLabel(QWidget* parent = nullptr)`：构造函数，支持设置父控件；
- `~ScrollingLabel()`：析构函数，用于释放 UI 资源；
- `void setText(const QString& text)`：用于设置当前控件显示的文本内容；
- `void resizeEvent(QResizeEvent*) override`：当控件尺寸变化时触发，内部调用 `updateScroll()` 判断是否需要重新启动滚动。

#### 四、私有槽函数

- `void scrollText()`：定时器超时时触发的槽函数，主要完成 offset 偏移值的更新，并通过重绘达到滚动效果；
- `void updateScroll()`：该函数用于判断当前文本是否需要滚动，如果文本宽度大于控件宽度，则启动滚动定时器，否则显示静态文本。

#### 五、私有成员

- `Ui::ScrollingLabel* ui`：该指针指向该控件的 UI 设计界面（通常用于配合 Qt Designer 的 .ui 文件）；
- `QTimer timer`：负责驱动文本滚动动画的定时器；
- `int offset`：记录当前文本的绘制偏移量，初始为 0，每次定时器触发时增加或重置。

## 源码分析

首先，构造函数 `ScrollingLabel::ScrollingLabel` 主要完成三件事：

1. 使用 `ui->setupUi(this)` 初始化界面（你应该在 Qt Designer 中设计了一个 QWidget，并放了一个 QLabel）。
2. 将 `QTimer::timeout` 信号连接到槽函数 `scrollText`，用于后续定时驱动。
3. 设置 QLabel 初始位置为 `(0, 0)`，确保文字从最左边开始。

构造完成后，接下来是设置阶段。

`setText` 函数用于设定新的滚动文字，编程思路是这样的：

1. 通过 `ui->label->setText(text)` 设置显示文本；
2. 使用 `adjustSize()` 自动调整 QLabel 的宽度，以确保文字完整；
3. 重置 `offset = 0`；
4. 调用 `updateScroll()` 决定是否启用滚动。

`updateScroll` 是一个逻辑判断函数。它的编写思路很简单直接：通过比较 `label` 宽度和当前控件宽度来判断是否需要滚动：

- 如果 QLabel 的宽度小于等于控件宽度，就不滚动，调用 `timer.stop()`；
- 否则启动定时器，让它不断调用 `scrollText`。

这个逻辑放在单独函数里，而不是和 `setText` 混在一起，是一种**解耦设计的体现**，便于其他场合也能直接调用 `updateScroll()`，比如控件尺寸变化时。

`scrollText` 是定时器定期调用的滚动处理函数。它的思路是：

1. 每次调用就将 `offset -= STEP_SHORT`，表示 label 向左移动几个像素；
2. 如果文字的尾部已经完全滚出（`offset + label宽度 < 0`），就把 offset 重置为控件宽度，相当于让文字从右边重新出现；
3. 最后调用 `ui->label->move(offset, 0)`，将 label 更新到新的位置。

 `resizeEvent`每次窗口尺寸变化时都调用 `updateScroll()`，确保新的尺寸下逻辑仍然正确。

整个类内部没有使用多余的信号槽、动画、lambda 或复杂样式，而是采用了 QTimer 和 QLabel 的组合，通过“修改位置”的方式控制滚动，实现上追求的是**简洁、直接、稳定**。用定时器而不是 `QPropertyAnimation`，也说明你有意识地避免不必要的资源开销或依赖，显然这是考虑到了运行环境对效率的需求。

这种风格在嵌入式 UI 或资源受限场景中尤其合适。