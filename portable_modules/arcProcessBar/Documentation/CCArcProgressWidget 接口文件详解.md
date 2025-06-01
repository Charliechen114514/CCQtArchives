# 如何在Qt中绘制一个弧形的进度条

在图形用户界面开发中，进度指示控件（Progress Widget）是非常常见且实用的组件。`CCArcProgressWidget` 是一个继承自 `QWidget` 的自定义控件，用于绘制圆弧形进度条。当然，笔者看了眼公开的实现，基本上都非常完善了，笔者在这里添加了一个更好的动画。

## 类定义概览

`CCArcProgressWidget` 类定义在 `CCArcProgressWidget.h` 中，使用 Qt 元对象系统，通过 `Q_OBJECT` 宏启用信号与属性机制。该控件支持如下属性绑定：

- `value`：当前进度值
- `maxValue`：最大进度值
- `displayValue`：动画过程中的显示值（与实际 `value` 异步）

这些属性可被 QML 或动画机制绑定，便于动态效果的呈现。（笔者这里使用的是Q_PROPERTY属性系统公开的，所以QML可用（笑））

## 静态常量定义（这部分是笔者认为编译的时候可以指定的）

类中定义了多个静态常量，用于控制组件的外观与行为：

- `DURATION`：动画持续时间（单位：毫秒），默认为 500ms
- `ARC_WIDTH`：圆弧的线宽，默认为 50 像素
- `DEFAULT_VALUE`：默认初始值，为 0
- `DEFAULT_MAX`：默认最大值，为 100
- `DEF_TEXT_COLOR`：默认文本颜色
- `DEF_BKCOLOR`：默认背景弧颜色（未完成部分）
- `DEF_ARC_COLOR`：默认进度弧颜色（已完成部分）

这些常量使得控件具有清晰的默认状态，便于使用和维护。

## 属性访问与设置接口

该类提供了一系列 `inline` 内联函数和公开接口，用于读取与设置进度值及外观样式：

- `int value() const`：获取当前进度值
- `void setValue(int val)`：设置当前进度值（含动画）
- `int maxValue() const`：获取最大值
- `void setMaxValue(int max)`：设置最大值
- `QColor progressArcColor() const` / `void setProgressArcColor(const QColor&)`：读取与设置进度弧颜色
- `QColor progressBackgroundColor() const` / `void setProgressBackgroundColor(const QColor&)`：读取与设置背景弧颜色
- `QColor progressTextColor() const` / `void setProgressTextColor(const QColor&)`：读取与设置文本颜色

所有设置函数内部均会判断是否真正发生变化，避免无谓的刷新，若发生更改则调用 `update()` 触发重绘。

## 信号机制

该控件定义了三个信号：

- `valueChanged(int)`：当用户设置新进度值时发出
- `maxValueChanged(int)`：当最大值被重新设置时发出
- `displayValueChanged(int)`：当动画中显示的值发生变化时发出

这些信号便于其他模块（如界面展示、数据记录）实时响应进度的变化。

## 绘制函数与动画支持

该类重载了 `paintEvent` 事件处理函数，实现核心绘制逻辑。绘制内容包括三部分：

- 背景弧：通过 `drawBackgroundArc()` 绘制未完成部分
- 进度弧：通过 `drawProgressArc()` 根据当前动画角度绘制完成部分
- 中心文本：通过 `drawText()` 绘制当前数值或状态文字

同时，`setupAnimation()` 函数用于构建 `QPropertyAnimation` 动画，使 `value` 到 `displayValue` 之间具备平滑过渡效果。动画期间实际值不变，仅 `displayValue` 动态变化，从而提升用户体验。

## 私有成员变量

类中使用了如下私有成员保存状态：

- `progress_value`：当前进度值
- `progress_display_value`：当前显示值（用于动画）
- `progress_max_value`：最大进度值
- `progress_minAngle` 与 `progress_startAngle`：控制弧线的起始与方向（默认从顶部顺时针）
- `progress_arc_color`、`progress_backgroundColor`、`progress_textColor`：颜色配置
- `QPropertyAnimation* animation`：动画对象指针

这些成员变量共同构成了进度显示的完整状态。

## 使用示例（简要）

```cpp
CCArcProgressWidget* widget = new CCArcProgressWidget(this);
widget->setValue(70);
widget->setMaxValue(100);
widget->setProgressArcColor(Qt::blue);
widget->setProgressBackgroundColor(Qt::lightGray);
widget->setProgressTextColor(Qt::black);
```

以上代码将在界面中创建一个蓝色的圆形进度条，表示当前进度为 70%。

## 一些实现的细节说明

​	下面的部分是属性设置的接口，没什么有趣的。

```c++
#include "CCArcProgressWidget.h"
#include <QPropertyAnimation>

CCArcProgressWidget::CCArcProgressWidget(QWidget* parent)
	: QWidget { parent } {
	setupAnimation();
}

void CCArcProgressWidget::setupAnimation() {
	animation = new QPropertyAnimation(this, "displayValue");
	animation->setDuration(DURATION);
	animation->setEasingCurve(QEasingCurve::OutCubic);
}

void CCArcProgressWidget::setValue(int val) {
	val = qBound(0, val, progress_max_value);
	if (val == progress_value) // avoid duplicate animations
		return;

	progress_value = val;
	animation->stop();
	animation->setStartValue(progress_display_value);
	animation->setEndValue(progress_value);
	animation->start();
}
```

​	下面说下我们的绘制，这里是每一次触发重绘的时候我们的设备实际上进行的绘制。

```c++
void CCArcProgressWidget::paintEvent(QPaintEvent* event [[maybe_unused]]) {
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);

	QRectF baseRect = rect();

	double side = qMin(baseRect.width(), baseRect.height());
	// 到这里，是为了获取绘制成正方形而不是椭圆形（不然太难看了）
	QRectF squareRect((baseRect.width() - side) / 2.0,
					  (baseRect.height() - side) / 2.0,
					  side, side);
    
	int margin = ARC_WIDTH + 5;
	QRectF arcRect = squareRect.adjusted(margin, margin, -margin, -margin);
	double radius = qMin(arcRect.width(), arcRect.height()) / 2;
	QPointF center = arcRect.center();

	double angle = 360.0 * progress_display_value / progress_max_value;
	angle = qMax<double>(progress_minAngle, -angle);

	drawBackgroundArc(painter, arcRect);
	drawProgressArc(painter, arcRect, angle);
	drawText(painter, center, radius);
}
```

1. `paintEvent` 事件首先确定绘制区域 `arcRect`，再根据当前 `displayValue` 计算对应的角度 `angle`。之后，依次调用：

- `drawBackgroundArc`：用圆弧绘制背景轨迹。
- `drawProgressArc`：绘制当前进度的圆弧，同时在圆弧末端绘制小圆点，增强视觉效果。
- `drawText`：居中绘制当前进度的百分比文本。

```c
void CCArcProgressWidget::drawBackgroundArc(QPainter& painter, const QRectF& arcRect) {
	QPen pen(progress_backgroundColor, ARC_WIDTH);
	pen.setCapStyle(Qt::RoundCap);
	painter.setPen(pen);
	painter.drawArc(arcRect, progress_startAngle * 16, 360 * 16);
}

void CCArcProgressWidget::drawProgressArc(QPainter& painter, const QRectF& arcRect, double angle) {
	if (angle == 0)
		return;
	QConicalGradient gradient(arcRect.center(), progress_startAngle);
	gradient.setColorAt(0, progress_arc_color.lighter(150));
	gradient.setColorAt(0.5, progress_arc_color);
	gradient.setColorAt(1, progress_arc_color.darker(150));

	QPen pen(QBrush(gradient), ARC_WIDTH);
	pen.setCapStyle(Qt::FlatCap);
	painter.setPen(pen);

	painter.drawArc(arcRect, progress_startAngle * 16, -angle * 16);
	double spanAngleRad = qDegreesToRadians(progress_startAngle - angle);
	double cx = arcRect.center().x();
	double cy = arcRect.center().y();
	double rx = arcRect.width() / 2;
	double ry = arcRect.height() / 2;
	double ex = cx + rx * qCos(spanAngleRad);
	double ey = cy - ry * qSin(spanAngleRad);

	QBrush brush(gradient);
	painter.setBrush(brush);
	painter.setPen(Qt::NoPen);
	painter.drawEllipse(QPointF(ex, ey), ARC_WIDTH / 2.0, ARC_WIDTH / 2.0);
}

void CCArcProgressWidget::drawText(QPainter& painter, const QPointF& center, double radius) {
	painter.setFont(QFont("Arial", radius * 0.3, QFont::Bold));
	painter.setPen(progress_textColor);

	QString text = QString("%1%").arg(qRound(100.0 * progress_display_value / progress_max_value));
	QRectF textRect(center.x() - radius, center.y() - radius,
					radius * 2, radius * 2);
	painter.drawText(textRect, Qt::AlignCenter, text);
}

```

