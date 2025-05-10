# 逐步理解Qt信号与槽机制1

## 前言

​	笔者几乎写了三年的Qt，每一年，我都写过Qt信号与槽的文章，但是笔者每一年都会在坚持的Qt实践中，加深对Qt信号与槽机制的理解。这里，笔者打算借助一些博客，我过去的一些经验，继续整理和消化一部分内容，把他放置到这里。供各位参考

## 从我们的编程实战出发

​	我们写Qt，实际上很多时候都是跟Qt的信号与槽打交道，或者说，**只要我们的代码写上了任何基于QObject的派生类，同时，我们还使用了QObject的connect和其他因此派生的方法的时候，我们就在使用Qt的信号与槽机制。**大家使用Qt信号与槽，用稍微专业的术语来讲，就是看中了它好用的异步机制。我们不需要关心当一个我们期待的信号被触发的时候，对应的动作被以一种类似回调的方式进行了触发回应。就像你在使用QScrollArea的时候不会绞尽脑汁的想如何让其他部分的模块感知到QScrollArea的滑条发生了改变，所有的参数和信息被按照一种魔法的方式，广播到了注册响应这个信号的所有对象上。这就是QObject好用的地方所在。对于上位机的开发中，我们一般理解到这里，把这个机制看成一个黑箱子，这样便足够了。

​	但是你来到了这篇文章，说明你不满足于此。我也是，为了让我们的重温一下，我写了一个最小的demo：

```c++
#include <QCoreApplication>
#include <QObject> /* Including for the QObject */
#include <QTimer> /* for a later invoke of set value and quit program */
/* For us, we always inherit QObejct to make
 * the class owns the property of QObject,
 * thus, we can further use the QObject's features.
 * now we are considering the signals and slots
 */
class MyObject : public QObject {
    Q_OBJECT
public:
	MyObject(QObject* parent = nullptr)
		: QObject(parent) {
		// Constructor code here
	}
	/* nothing special */
	~MyObject() = default;

	void set_value(int _value) {
		value = _value;
		/* critical points: broadcast the signals */
		emit valueChanged(value);
	}
signals:
	/*
	 * every time, the signal indicate the value
	 * the object current holding has been changed
	 * thus, any other objects observing this special
	 * event will get to know now or later
	 */
	void valueChanged(int value);

private:
	int value;
};

class MyHook : public QObject {
	Q_OBJECT
public:
	MyHook(QObject* parent = nullptr)
		: QObject(parent) {
		// Constructor code here
	}
	~MyHook() = default;
	/* nothing special */
public slots:
	void handle_value_changed(int value) {
		qInfo() << "[MyHook]: get to know the value changed to" << value;
	}
};

int main(int argc, char* argv[]) {
	QCoreApplication a(argc, argv);
	MyObject* obj = new MyObject();
	MyHook* hook = new MyHook();

	/* connect the signal and slot */
	/* when the signal is emitted, the slot will be called */
	QObject::connect(obj, &MyObject::valueChanged,
					 hook, &MyHook::handle_value_changed);

	QTimer::singleShot(1000, obj, [obj]() {
		obj->set_value(42); /* we set value after the program launched 1s */
	});

	/* quit the program later */
	QTimer::singleShot(2000, &a, [&a, obj, hook]() {
		delete obj;
		delete hook;
		qInfo()
			<< "[Main]: quitting the program...";
		a.quit();
	});

	return a.exec();
}

#include "main.moc"

```

​	这个代码做的事情非常的简单：我首先创建了两个对象（在堆上，这样方便我写信号与槽的连接），将obj的信号和hook的处理槽相连接，在程序运行的第一个秒时，笔者让obj的值发生一点变化，比如说让他的值设置成了42.这个时候，笔者封装的set_value除了简单的将值传递给了value之外，还发射了一个信号：emit valueChanged，把值携带了出去，当然，为了让程序可以退出不被卡死，笔者自己有设置了程序运行的第二秒退出去了（这个跟我们研究的东西没有关系）。

​	你很快注意到，我们在下面，又引入了一个奇怪的东西，那就是main.moc。这个东西我们很快就会知道是干啥的了——对了，你大可不必现在焦急的找这个main.moc文件了（如果您的检查器在摇你找不到这个文件，那就去他大坝的），在没有触发MOC，也就是Meta Object Compiler对您的C++文件做扫描之前，你不会找到哪怕一个字眼的main.moc！

​	点击QtCreator的运行按钮

```
[MyHook]: get to know the value changed to 42
[Main]: quitting the program...

Process exited with code: 0.
```

​	这是我们意料之内的结果，你耸耸肩，这没什么，对吧。

## QObject和Q_OBJECT

​	需要注意的是，我们的信号与槽，其由于我们额外的MOC的引入（现在如果你一头雾水，请你假装这是Qt信号与槽魔法的有力保证者，它用无形的大手真把信号与槽连接起来了），信号与槽机制成为了实际上Qt对C++的扩展功能了。我们如果想要使用元对象，信号与槽等Qt对C++扩展的功能，**就务必让我们亲爱的public QObject和Q_OBJECT**同时出现在一起。当然，**如果你不用，单纯的就是想使用QObject的一些对象性质的属性（嗯？有从Java来的嘛？Object类！），去掉它，你的类会很轻的！**

​	QObject本身很复杂，它的特性我们亲爱的Qt文档写了好几个大章节。我们这里只关心它的信号与槽的侧面。可以这样说，QObject在这里，本身就提供了一个对外的信息——我可以被使用信号与槽机制，但是我只能说我可以。至于能不能，需要下一位重量级选手Q_OBJECT来提供我们真正对Qt扩展的C++（比如说元对象啊信号与槽乱七八糟）的支持。

​	这是因为，只有当我们的类中，Q_OBJECT出现在了里面，我们的MOC编译器才会施展魔法，把我们的类注册进信号与槽的大系统里。Q_OBJECT自身也是对一些类RTTI信息的薄记（当然比经典C++强大的多！）。

```c++
#define Q_OBJECT \
public: \
    QT_WARNING_PUSH \
    Q_OBJECT_NO_OVERRIDE_WARNING \
    static const QMetaObject staticMetaObject; \
    virtual const QMetaObject *metaObject() const; \
    virtual void *qt_metacast(const char *); \
    virtual int qt_metacall(QMetaObject::Call, int, void **); \
    QT_TR_FUNCTIONS \
private: \
    Q_OBJECT_NO_ATTRIBUTES_WARNING \
    Q_DECL_HIDDEN_STATIC_METACALL static void qt_static_metacall(QObject *, QMetaObject::Call, int, void **); \
    QT_WARNING_POP \
    struct QPrivateSignal { explicit QPrivateSignal() = default; }; \
    QT_ANNOTATE_CLASS(qt_qobject, "")
```

​	很好，一点没有让人看的欲望，我放在这里，是因为有人可能对我上面说的**Q_OBJECT自身也是对一些类RTTI信息的薄记**表达疑问。这里我们的QMetaObject等方法和薄记的类信息共同构成了Q_OBJECT，当然，这也是为什么QOBJECT放在前面后我们自己写的方法一般还需要手动加public，因为它的下面是private。不然的话，你所有的方法的可见性全部成了private。

## MOC编译器

​	我们的工作，就是简单的，把期望使用Qt信号与槽机制的对象，先注册成QObject的子类，标注激活MOC扫描的O_OBJECT，以及猛猛的connect signals and slots。嗯，我们就是这样用的。但是这里一切的魔法的基础，是我们的MOC编译器。

​	笔者翻阅了Qt的文档，大概得到的结论是这样的。

​	MOC编译器分为三个阶段进行工作：

- 代码扫描
- 生成扩展代码
- 编译与链接

### 代码扫描

​	是的，不然我们的Qt信号与槽大系统，咋知道你的信号与槽是谁对谁呢？MOC编译器读取**C++头文件**并查找所有包含**Q_OBJECT**宏的类声明，同时识别signals、slots、Q_PROPERTY和Q_INVOKABLE等关键字；若在.cpp文件内定义了这些宏则默认不生成对应moc文件，除非显式包含扫描的结果，你看，这就解答了我们的之前的——main.moc的作用。我们所有的对信号与槽接口的支持，全部就在这里做到的。

### 生成扩展代码

​	这样的moc文件，显然是要写东西的，我们要把我们代码扫描得到的一些结论写入我们的MOC文件中。MOC编译器在这个MOC文件中就定义好了静态成员QMetaObject结构、槽转发函数、信号发射器以及属性读写函数等。该文件实现了metaCall()方法，用于在运行时通过枚举索引调用对应的槽或方法。这也是我们之后分析的重点了。也是我们揭晓信号与槽神秘机制的一个大的要点。

### 编译与链接

​	这一部分内容，会被一起送到构建系统中，在QMake时代，这个是QMake自己做的统计，CMake需要使用Qt对CMake函数的封装。这里，Moc文件会被认为是C++源代码的一部分，一起被送到标准的编译器一起编译。

## 所以，我们的moc文件里，写了什么（信号与槽的静态分析）

​	我们现在，分析一份躺在磁盘上的MOC文件。这是文件的开头

```
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif

#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'main.cpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif
```

​	笔者这里使用的是Qt6.6.3，当然，你的版本大概率和我的不一样，因此这里的内容也会有所差异。这样也就意味着MOC是不可跨版本的，更换Qt版本，项目也就必须被重新编译（换而言之，MOC是不可复用的）

### 字符串数据

```c++
#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSMyObjectENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSMyObjectENDCLASS = QtMocHelpers::stringData(
    "MyObject",
    "valueChanged",
    "",
    "value"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSMyObjectENDCLASS_t {
    uint offsetsAndSizes[8];
    char stringdata0[9];
    char stringdata1[13];
    char stringdata2[1];
    char stringdata3[6];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSMyObjectENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSMyObjectENDCLASS_t qt_meta_stringdata_CLASSMyObjectENDCLASS = {
    {
        QT_MOC_LITERAL(0, 8),  // "MyObject"
        QT_MOC_LITERAL(9, 12),  // "valueChanged"
        QT_MOC_LITERAL(22, 0),  // ""
        QT_MOC_LITERAL(23, 5)   // "value"
    },
    "MyObject",
    "valueChanged",
    "",
    "value"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSMyObjectENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   20,    2, 0x06,    1 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,

       0        // eod
};
```

​	这里就是薄记，谁是谁的一个重要的地方。我们字符串元数据布局的存在是为了将所有与元对象相关的字符串（类名、信号名、槽名、属性名等）集中到一块连续的只读内存中，通过一个 offsetsAndSizes 数组记录每条字符串在这块缓冲区里的偏移和长度，这样既节省空间又方便运行时通过一个统一指针访问任意字符串，而不需要为每个字面量生成单独的常量，这正是 moc 在生成 qt_meta_stringdata_… 结构体时所做的工作，它输出一个 uint offsetsAndSizes[N] 和紧随其后的字符数组，借助 QtMocHelpers::stringData 或手动布局完成这一设计，从而在 QMetaObject 引用时只需提供 offsetsAndSizes 即可定位任意名称字符串。这样，我们找东西（具体咋找，是放在后面讲解的），就能找到这个由元数据数组与 QMetaObject 把类的元信息（moc 版本号、类名索引、方法数与偏移、属性数、枚举数、信号数等）以整型数组形式编码，这个数组在 staticMetaObject 中被引用，从而构成完整的元对象描述。QMetaObject 包含指向父类 staticMetaObject 的 SuperData 链接，以及对该字符串元数据和元数据数组的指针，再加上指向 qt_static_metacall 的分发入口和未完成类型数组，运行时通过 QObject::metaObject() 返回这个结构，就能查询到任意 QObject 子类的全部信号、槽、属性和方法签名等

```c
Q_CONSTINIT const QMetaObject MyObject::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSMyObjectENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSMyObjectENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSMyObjectENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MyObject, std::true_type>,
        // method 'valueChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };
```

​	当我们注册好了元对象的信息和信号与槽之后，我们就给嵌入在类内的staticMetaObject赋值做初始化了（对的，你可以看到Q_OBJECT的staticMetaObject就是在这里起到作用的）

### 转化信号与槽为可异步的成员函数调用

​	我们的qt_static_metacall做的工作，就是打标检索信号与槽，把对应的信号绑定槽函数回调起来：

```
void MyObject::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) { /* 直接调用元方法 */
        auto *_t = static_cast<MyObject *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {	/* 打表办法 */
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MyObject::*)(int );
            if (_t _q_method = &MyObject::valueChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
}
```

​	qt_static_metacall 动态方法分发的作用在于**将外部统一的元调用（QMetaObject::Call）转换为具体的 C++ 成员函数调用。无论是发射信号（InvokeMetaMethod）、查询方法索引（IndexOfMethod）还是注册参数类型（RegisterMethodArgumentMetaType），都通过传入同一个 qt_static_metacall 函数**，加上方法索引和 void** 参数数组，由 switch 语句或者比较成员**指针的方式定位到对应的成员函数并执行**，从而把所有信号槽和可调用方法在运行期以一种极简的“索引＋参数表”机制实现解耦分发，天然支持跨线程的 QueuedConnection 和动态 invoke。

```
/* 实现一些元对象系统的接口，这里跟信号与槽关系没有很大 */
const QMetaObject *MyObject::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MyObject::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSMyObjectENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

/* 	这里就是qt_static_metacall的前端了，
	对于信号，走的是RegisterMethodArgumentMetaType，将对应的位置注册上
	对于回调方法的槽，信号与槽的机制，走的就是InvokeMetaMethod 
*/
int MyObject::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void MyObject::valueChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
```

​	最后 metaObject、qt_metacast 和 qt_metacall 三者协同工作以支持动态类型识别、跨类转换与分发调用。metaObject() 在没有动态扩展时返回 staticMetaObject，否则返回 dynamicMetaObject，保证运行时新增属性或继承扩展依然可见。qt_metacast 根据传入类名字符串与本类元数据中的类名比较，命中则返回 this 实现 qobject_cast 的底层类型安全转换，否则委托给父类；qt_metacall 则在 QObject 基类处理完通用逻辑后，将剩余的 InvokeMetaMethod 或 RegisterMethodArgumentMetaType 调用传给 qt_static_metacall 执行具体方法，并调整索引以支持继承链上多级方法分发，这样就完成了从字符串／索引到具体 C++ 方法的全流程动态调用。在 MyObject::valueChanged(int) 中，moc 生成了一个本地数组 _a ，第一个元素留空（占位 this 指针），第二个元素存放参数地址，然后调用 QMetaObject::activate 将信号索引 0 以及参数数组打包并分发到所有连接的槽函数。 QMetaObject::activate 会检查连接类型（DirectConnection、QueuedConnection 等），并在必要时将调用封装为事件投递到目标对象线程。

​	MyHook 类重复了上述流程，这里笔者简单的说下区别：由 qt_meta_stringdata_CLASSMyHookENDCLASS 和 qt_meta_data_CLASSMyHookENDCLASS 定义字符串与元数据数组， staticMetaObject 链接 QObject 父元数据，并生成 qt_static_metacall、metaObject、qt_metacast、qt_metacall 等函数。唯一差异在于 signalCount 为 0 而 methodCount 为 1，将“handle_value_changed(int)”注册为槽而非信号，moc 也据此在 qt_static_metacall 中仅实现 InvokeMetaMethod 分支，以调用 MyHook::handle_value_changed(int)

## 所以，信号与槽机制工作的时候，到底发生了怎样的流程？（信号与槽的动态分析）

​	在你已经准备打瞌睡的时候，我们打算进行动态的分析，让这套流程在时序的工作流程得以明确：当我们 `emit valueChanged(value)` 这一行代码时，moc 为 `valueChanged` 信号生成的隐藏成员函数首先在栈上构造了一个 `void*` 数组 `_a`，其第一个元素始终为 `nullptr`（占位 `this` 指针），第二个元素**存放着参数 `value` 的地址**。紧接着，这个隐藏函数会调用 `QMetaObject::activate(this, &staticMetaObject, 0, _a)`，其中 `0` 是信号在本类元数据中的本地索引（local_signal_index），`staticMetaObject` 则携带了所有信号、槽和属性的元信息。

### QMetaObject::activate 如何查找并分发连接

在 `QMetaObject::activate` 内部，首先会通过 `QMetaObjectPrivate::signalOffset()` 将**本地索引转换为全局信号索引**（signal_index），确保继承链上所有父类信号都能被正确定位。随后，`activate` 版本中带有 `signalOffset` 和 `local_signal_index` 的重载函数会遍历该信号对应的连接列表（connectionLists），对于每一条连接节点，它要么直接调用目标槽（DirectConnection），要么将调用封装为事件放入目标对象所属线程的事件队列（QueuedConnection）以实现跨线程安全。

在 DirectConnection 情况下，`activate` 会立即通过内部的元调用机制 `qt_metacall(receiver, InvokeMetaMethod, methodIndex, argv)` 将控制权交给接收者对象；在 QueuedConnection 情况下，则生成一个事件对象，延迟到接收者线程的事件循环中再由其事件调度器派发执行，确保线程安全性与无锁并发。（QueueConnections则会投放到事件队列异步回调）

### 接收者端的 qt_metacall 与 qt_static_metacall

一旦到达接收者端，首先执行的是 `QObject::qt_metacall(_c, _id, _a)`，它会处理框架内部的通用调用逻辑并返回调整后的方法索引；若该索引仍在本类范围内，则会转入由 moc 生成的 `qt_static_metacall(this, _c, _id, _a)` 中继续处理。在 `qt_static_metacall` 内部，通过对 `InvokeMetaMethod` 类型和索引值进行 `switch` 分支判断，最终定位到 `MyHook::handle_value_changed(int)` 并以原始参数调用该成员函数，实现信号到槽的最终映射。

在槽函数 `handle_value_changed` 中，用户代码使用 `qInfo()` 输出日志，至此一次完整的信号—槽调用链从发射到执行告一段落。整个过程中，静态元对象 `staticMetaObject` 提供了必要的类名、方法签名和参数类型信息，`QMetaObject::activate` 则负责连接查找与调用分发，`qt_metacall`/`qt_static_metacall` 则完成索引到实际成员函数的跳转。

# Reference

-  Qt 官方文档 “The Meta‑Object System” ([Qt 文档](https://doc.qt.io/qt-6/metaobjects.html?utm_source=chatgpt.com))
-  Woboq 博客 “How Qt Signals and Slots Work” ([Woboq - We Create Software](https://woboq.com/blog/how-qt-signals-slots-work.html?utm_source=chatgpt.com))
-  Qt Wiki “QObject‑Internals” ([Qt维基](https://wiki.qt.io/QObject-Internals?utm_source=chatgpt.com))
-  StackOverflow “How does Qt implement signals and slots?” ([Stack Overflow](https://stackoverflow.com/questions/2008033/how-does-qt-implement-signals-and-slots?utm_source=chatgpt.com))
-  StackOverflow “What does the Q_OBJECT macro do?” ([Stack Overflow](https://stackoverflow.com/questions/1368584/what-does-the-q-object-macro-do-why-do-all-qt-objects-need-this-macro?utm_source=chatgpt.com))
-  Medium “Understanding Qt::QueuedConnection” ([Medium](https://medium.com/@alphaneker40/understanding-qt-queuedconnection-a-deep-dive-into-signals-and-slots-in-multithreaded-qt-62184ca5722f?utm_source=chatgpt.com))
-  NTCore “Qt Internals & Reversing” ([NTCore](https://www.ntcore.com/files/qtrev.htm?utm_source=chatgpt.com))
-  Qt 源码中的 QMetaObject 定义 ([Qt 文档](https://doc.qt.io/qt-5/qmetaobject.html?utm_source=chatgpt.com))
-  Digital Fanatics “The Qt Object Model” ([digitalfanatics.org](https://www.digitalfanatics.org/projects/qt_tutorial/chapter02.html?utm_source=chatgpt.com))
-  MIT 教程 “Signals and Slots” ([Massachusetts Institute of Technology](https://web.mit.edu/~firebird/arch/i386_rhel3/doc/html/signalsandslots.html?utm_source=chatgpt.com))