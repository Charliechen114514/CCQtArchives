# 如何使用Qt创建一个浮在MainWindow上的滑动小Panel

## 前言

​	之前笔者去其他城市做了点事情，现在回到家中可以好好的扩展一下自己的Qt的功能库。笔者最近正在给我的小日历写一个超级简单的浮动的小窗口，源码中就是一个简单的小PanelWidget，比起来更加像是一种静态的可继承的扩展库（Floatable Panel Property），注意到的朋友可以自行继承PanelWidget，无论是带有Ui文件者还是其他均可复用。

## 分析

​	PanelWidget作为一个更加像是模板的Widget，实际上中心思路非常简单：那就是将PanelWidget自身设置成为SubWindow，这个SubWindow总是对齐于ParentWindow（主窗口）进行坐标系的参考（不然的话他就会总是出现在Screen坐标的最左边或者云云）

> setWindowFlags实际上设置的是QtWidgets的一些特性，当然，我看到最经常大伙用的就是经典的FrameLessWindow的flags，这里我们使用SubWindow来重新调整我们的Qt的计算的坐标系。

​	我们的期望是这样的——我们的ParentWindow动的时候，我们的PanelWidget显然也要跟随移动，但是因为他不在Layout中，我们就要自己监听我们的ParentWindow来做出对应的举措，为此，installEventFilter是我们需要做的（嗯，监听我们的夫窗口）

```cpp
bool PanelWidget::eventFilter(QObject* watched, QEvent* event) {
    if (watched == parentWidget() && (event->type() == QEvent::Move || event->type() == QEvent::Resize)) {
        updatePosition();	///< 在这里，我们重新计算我们的位置
    }
    return QWidget::eventFilter(watched, event);
}
```

​	重新计算是非常简单的，我们的Widget在静态的时候无非就是两个

```cpp
void PanelWidget::updatePosition() {
    if (!parentWidget())
        return;

    QRect parentRect = parentWidget()->geometry();
    QRect targetRect(0, 0, width(), parentRect.height());

    if (isVisible() && !isSlidingIn) {
        setGeometry(targetRect);	///< 看得见的时候，放置到可以看到的左侧栏上
    } else {
        setGeometry(targetRect.translated(-width(), 0)); ///< 反之，放置到右边刚好跟主窗口对齐的位置上
    }
}
```

​	我们还需要滑动的特效，看过我博客的兄弟们都知道：`QPropertyAnimation`总是一个非常好的选择，这里我们就是对Geometry做这个东西玩，当然，这里就直接抽象两个重要的接口函数

```cpp
void PanelWidget::slideIn() {
    isSlidingIn = true;

    updatePosition();
    show();
    raise();

    animation->stop();
    animation->setStartValue(geometry());
    animation->setEndValue(QRect(0, 0, width(), height()));

    disconnect(animation, &QPropertyAnimation::finished, this, &PanelWidget::closePanel);
    animation->start();

    connect(animation, &QPropertyAnimation::finished, this, [this]() {
        isSlidingIn = false;
    });
}

void PanelWidget::slideOut() {
    animation->stop();
    animation->setStartValue(geometry());
    animation->setEndValue(QRect(-width(), 0, width(), height()));
    connect(animation, &QPropertyAnimation::finished, this, &PanelWidget::closePanel);
    animation->start();
}

void PanelWidget::closePanel() {
    disconnect(animation, &QPropertyAnimation::finished, this, &PanelWidget::closePanel);
    hide();
}
```

## 源代码

```cpp
#include "PanelWidget.h"
#include <QEvent>
#include <QPropertyAnimation>
PanelWidget::PanelWidget(QWidget* parent)
    : QWidget { parent } {
    animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(300);

    setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_DeleteOnClose, false);
    parent->installEventFilter(this);
}

void PanelWidget::updatePosition() {
    if (!parentWidget())
        return;

    QRect parentRect = parentWidget()->geometry();
    QRect targetRect(0, 0, width(), parentRect.height());

    if (isVisible() && !isSlidingIn) {
        setGeometry(targetRect);
    } else {
        setGeometry(targetRect.translated(-width(), 0));
    }
}

bool PanelWidget::eventFilter(QObject* watched, QEvent* event) {
    if (watched == parentWidget() && (event->type() == QEvent::Move || event->type() == QEvent::Resize)) {
        updatePosition();
    }
    return QWidget::eventFilter(watched, event);
}

void PanelWidget::slideIn() {
    isSlidingIn = true;

    updatePosition();
    show();
    raise();

    animation->stop();
    animation->setStartValue(geometry());
    animation->setEndValue(QRect(0, 0, width(), height()));

    disconnect(animation, &QPropertyAnimation::finished, this, &PanelWidget::closePanel);
    animation->start();

    connect(animation, &QPropertyAnimation::finished, this, [this]() {
        isSlidingIn = false;
    });
}

void PanelWidget::slideOut() {
    animation->stop();
    animation->setStartValue(geometry());
    animation->setEndValue(QRect(-width(), 0, width(), height()));
    connect(animation, &QPropertyAnimation::finished, this, &PanelWidget::closePanel);
    animation->start();
}

void PanelWidget::closePanel() {
    disconnect(animation, &QPropertyAnimation::finished, this, &PanelWidget::closePanel);
    hide();
}
```

```cpp
#ifndef PANELWIDGET_H
#define PANELWIDGET_H

#include <QWidget>
class QPropertyAnimation;
class PanelWidget : public QWidget {
	Q_OBJECT
public:
    /**
     * @brief PanelWidget
     * @param parent
     */
    explicit PanelWidget(QWidget* parent = nullptr);

    void slideIn(); ///< slide the widget in
    void slideOut(); ///< slide the widget out

private:
    QPropertyAnimation* animation; ///< Animation Holder
    bool isSlidingIn { false }; ///< marking for the sliding
    void closePanel(); ///< panel close IMPL
    void updatePosition(); ///< position calculation
    /**
     * @brief eventFilter
     * @param watched
     * @param event
     * @return
     */
    bool eventFilter(QObject* watched, QEvent* event) override;
};

#endif // PANELWIDGET_H
```

