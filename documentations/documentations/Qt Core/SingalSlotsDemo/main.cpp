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
