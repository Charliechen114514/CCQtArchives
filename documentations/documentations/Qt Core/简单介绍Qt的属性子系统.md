# 深入理解Qt的属性系统

​	笔者最近正在大规模的开发Qt的项目和工程，这里笔者需要指出的是，这个玩意在最常规的Qt开发中是相对比较少用的，笔者也只是在Qt的QPropertyAnimation需要动画感知笔者设置的一个属性的时候方才知道这个东西的。因此，这篇文章的属性更加象是整合和消化，请各位Qt大佬批评指正。

## 先说说Qt的属性系统可以做啥

​	我不想一上来就扔出来一大堆概念和写法，这没意思，看着啥也看不懂就来整一处，只会把人吓跑。

### 更加高级的和封闭的属性读写机制（C++的反射和动态属性）

​	在纯 C++ 环境下，通过 QObject::setProperty 和 QObject::property，开发者可以在不知道具体类定义的情况下，以字符串形式读写对象属性，从而实现对第三方或未知类型对象的动态操作，这在插件系统或运行时配置中尤为有用。如果需要在不同线程之间安全地调用方法，也可以借助 **QMetaObject::invokeMethod 配合 Qt::QueuedConnection**，将对属性的修改或方法调用排入目标线程的事件队列，从而避免手动管理互斥锁和死锁风险。这个就是把对对象属性的修改扔到巨大的事件循环队列中，事件循环队列是保证我们的数据读写不会发生争夺，必须按照顺序一个个执行的。

### UI 自动化与 Qt Designer 集成

​	Qt Designer 以及 Qt Quick Designer 均依赖元对象系统和属性系统来生成可视化属性面板。无论是 QWidget 还是 QML 项目，当在设计器中选中控件时，右侧属性编辑器都会列出该控件所有可设计属性，包括可脚本化（SCRIPTABLE）和可存储（STORED）的属性，通过动态读取 QMetaObject 数据实现即时更新和回显，这为界面原型设计和主题定制提供了极大便利。这个事情在笔者之后体验如何自己制作自己的插件的时候会进行尝试。



### QML 中的数据绑定与模型驱动（这个没做过，gpt这样说的）

在 QML 层面，属性系统是响应式界面的基石。将 C++ 对象通过 QQmlContext 或 qmlRegisterType 暴露给 QML 时，凡是使用 Q_PROPERTY 宏声明并带有 NOTIFY 信号的属性，都可以在 QML 中像普通属性一样使用绑定表达式。当模型层的属性在 C++ 侧被修改时，对应的 QML 界面会自动更新，无需手动编写额外的同步代码，这使得使用 Qt Quick 构建数据驱动型界面变得直观而高效。即便是在复杂的逻辑场景下，QML 引擎也会跟踪属性的依赖关系，在底层的 BindingEvaluator 中智能地避免多余的计算与重绘，从而兼顾性能和开发效率

### 动画、状态机与属性驱动

Qt 的动画框架广泛使用属性系统来驱动动画效果。QPropertyAnimation 类通过属性名称来获取和设置目标对象的属性值，并在不同时间点上插值更新，例如对 QWidget 的 geometry、opacity 或自定义属性进行平滑过渡；在状态机（QStateMachine）中，也可以使用属性动作（QPropertyAction）根据状态切换自动修改属性，从而将动画与状态逻辑解耦，简化状态驱动的界面行为实现。这个在笔者自己手搓网易云的动画效果的时候又遇到过，笔者会在那里详细的说明这个用途

## 非常心动，所以咋玩

​	很正常的想法！这个属性非常的有趣，所以我们需要怎么做呢？答案是下面的：

```
Q_PROPERTY(type name
           (READ getFunction [WRITE setFunction] |
            MEMBER memberName [(READ getFunction | WRITE setFunction)])
           [RESET resetFunction]
           [NOTIFY notifySignal]
           [REVISION int | REVISION(int[, int])]
           [DESIGNABLE bool]
           [SCRIPTABLE bool]
           [STORED bool]
           [USER bool]
           [BINDABLE bindableProperty]
           [CONSTANT]
           [FINAL]
           [REQUIRED])
```

​	注意，这些代码实际上啥也不会留下，他是给我们的MOC提供信息用的。告诉我们这个属性的性质，名称，如何读写等等方法。举个例子：

```
Q_PROPERTY(bool focus READ hasFocus)
Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)
Q_PROPERTY(QCursor cursor READ cursor WRITE setCursor RESET unsetCursor)
```

​	依次说明了如下的基本事实：

#### 1. `Q_PROPERTY(bool focus READ hasFocus)`

声明了一个名为 `focus` 的布尔类型属性：

- **READ**：通过 `hasFocus()` 方法读取属性值（无写入功能）。
- **用途**：通常表示控件是否获得键盘焦点（如输入框被选中时）。
- **特点**：这是一个只读属性，无法直接修改（没有 `WRITE` 或 `MEMBER` 标记）。

------

#### 2. `Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)`

声明了一个名为 `enabled` 的布尔类型属性：

- **READ**：通过 `isEnabled()` 方法读取属性值。
- **WRITE**：通过 `setEnabled(bool)` 方法修改属性值。
- **用途**：控制控件是否启用（禁用时通常变灰且不响应用户输入）。
- **特点**：支持读写，修改时会自动触发 Qt 的属性系统通知（如信号或样式更新）。

------

#### 3. `Q_PROPERTY(QCursor cursor READ cursor WRITE setCursor RESET unsetCursor)`

声明了一个名为 `cursor` 的 `QCursor` 类型属性：

- **READ**：通过 `cursor()` 方法获取当前光标样式。
- **WRITE**：通过 `setCursor(QCursor)` 方法设置光标样式（如鼠标悬停时变为手型）。
- **RESET**：通过 `unsetCursor()` 方法恢复默认光标（移除自定义设置）。
- **用途**：动态改变控件的光标外观。
- **特点**：支持读、写和重置操作，灵活性更高。

```
#include <QObject>
class Person : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
public:
    explicit Person(QObject *parent = nullptr) : QObject(parent) {}
    QString name() const { return m_name; }
    void setName(const QString &name) {
        if (m_name != name) {
            m_name = name;
            emit nameChanged();
        }
    }
signals:
    void nameChanged();
private:
    QString m_name;
};

// 在某处使用
Person p;
p.setProperty("name", "Alice");
QString current = p.property("name").toString();
```

​	当然，这样我们就完成了一次对属性的读写。这个的好处其实跟正常的get set接口类似，但是多了Qt上的很多元对象支持，就是我刚刚提到的那些。







## 附录：属性文档

- 如果未指定 MEMBER 变量，则需要一个 READ 访问器函数。该函数用于读取属性值。理想情况下，使用 const 函数来实现此目的，并且它必须返回属性的类型或该类型的 const 引用。例如，QWidget::focus 是一个只读属性，其 READ 函数为 QWidget::hasFocus()。如果指定了 BINDABLE 变量，则可以编写 READ default 来从 BINDABLE 变量生成 READ 访问器。
- WRITE 访问器函数是可选的。它用于设置属性值。它必须返回 void，并且必须只接受一个参数，该参数可以是属性的类型、指向该类型的指针或引用。例如，QWidget::enabled 具有 WRITE 函数 QWidget::setEnabled()。只读属性不需要 WRITE 函数。例如，QWidget::focus 没有 WRITE 函数。如果您同时指定了 BINDABLE 和 WRITE 的默认值，则系统会从 BINDABLE 生成一个 WRITE 访问器。生成的 WRITE 访问器不会显式发出任何使用 NOTIFY 声明的信号。您应该将该信号注册为 BINDABLE 的变更处理程序，例如使用 Q_OBJECT_BINDABLE_PROPERTY。
- 如果没有指定 READ 访问器函数，则需要 MEMBER 变量关联。这使得给定的成员变量无需创建 READ 和 WRITE 访问器函数即可读写。如果您需要控制变量访问，除了 MEMBER 变量关联之外，仍然可以使用 READ 或 WRITE 访问器函数（但不能同时使用）。
- RESET 函数是可选的。它用于将属性设置回其上下文相关的默认值。例如，QWidget::cursor 具有典型的 READ 和 WRITE 函数，即 QWidget::cursor() 和 QWidget::setCursor()，此外它还具有 RESET 函数，即 QWidget::unsetCursor()，因为任何对 QWidget::setCursor() 的调用都不能重置为上下文特定的游标。RESET 函数必须返回 void 且不接受任何参数。
- NOTIFY 信号是可选的。如果定义，则应指定该类中一个现有的信号，该信号在属性值发生变化时发出。MEMBER 变量的 NOTIFY 信号必须接受零个或一个参数，并且该参数必须与属性的类型相同。该参数将采用属性的新值。NOTIFY 信号应仅在属性确实发生更改时发出，以避免在 QML 中不必要地重新评估绑定。当通过 Qt API（QObject::setProperty、QMetaProperty 等）更改属性时，会自动发出信号，但直接更改 MEMBER 时则不会发出信号。
- REVISION 编号或 REVISION() 宏是可选的。如果包含，则它定义了在特定 API 版本（通常用于暴露给 QML）中使用的属性及其通知信号。如果不包含，则默认为 0。
- DESIGNABLE 属性指示该属性是否应在 GUI 设计工具（例如 Qt Widgets Designer）的属性编辑器中可见。大多数属性都是 DESIGNABLE（默认为 true）。有效值为 true 和 false。
- SCRIPTABLE 属性指示此属性是否应由脚本引擎访问（默认为 true）。有效值为 true 和 false。
- STORED 属性指示应将属性视为独立存在还是依赖于其他值。它还指示在存储对象状态时是否必须保存属性值。大多数属性都是 STORED 属性（默认为 true），但例如 QWidget::minimumWidth() 的 STORED 属性为 false，因为它的值取自 QWidget::minimumSize() 属性的宽度部分，而该属性的类型为 QSize。
- USER 属性指示该属性是被指定为面向用户的属性还是用户可编辑的属性。通常，每个类只有一个 USER 属性（默认为 false）。例如，QAbstractButton::checked 是（可勾选的）按钮的用户可编辑属性。请注意，QItemDelegate 可以获取和设置小部件的 USER 属性。
- BINDABLE bindableProperty 属性指示该属性支持绑定，并且可以通过元对象系统 (QMetaProperty) 设置和检查与该属性的绑定。bindableProperty 命名一个 QBindable<T> 类型的类成员，其中 T 是属性类型。此属性是在 Qt 6.0 中引入的。
- CONSTANT 属性的存在表示该属性值为常量。对于给定的对象实例，常量属性的 READ 方法每次调用时必须返回相同的值。对于不同的对象实例，此常量值可能不同。常量属性不能具有 WRITE 方法或 NOTIFY 信号。
- FINAL 属性的存在表示该属性不会被派生类覆盖。在某些情况下，这可以用于性能优化，但 moc 并不强制执行。必须注意切勿覆盖 FINAL 属性。
- REQUIRED 属性的存在表示该属性应由类的用户设置。moc 并不强制执行此操作，并且主要用于适用于暴露给 QML 的类。在 QML 中，除非设置了所有必需属性，否则无法实例化具有必需属性的类。