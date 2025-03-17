#ifndef CCMOVABLEWIDGET_H
#define CCMOVABLEWIDGET_H

#include <QWidget>
class CCMovableWidget : public QWidget {
    Q_OBJECT
public:
    // Constructor of the movable widget
    explicit CCMovableWidget(QWidget *parent = nullptr);

    void inline setMovingWidget(QWidget *widget) {
        holding_widget = widget;
        holding_widget->installEventFilter(this);
    }

    /*
        Buttons api accepts apis to set the acceptable mouse buttons
     */
    void inline installAcceptableMouseButtons(Qt::MouseButton button) {
        accept_buttons.insert(button);
    }

    void inline uninstallAcceptableMouseButtons(Qt::MouseButton button) {
        accept_buttons.remove(button);
    }

    void inline clearAcceptableMouseButtons() { accept_buttons.clear(); }

private:
    QWidget *holding_widget;
    void     handling_mousePressEvent(QMouseEvent *event);
    void     handling_mouseReleaseEvent(QMouseEvent *event);
    void     handling_mouseMoveEvent(QMouseEvent *event);
    // QWidget interface
private:
    struct {
        bool   pressed;       // indicates if the widget is pressed
        QPoint lastPoint;     // memorized the lastpoints
        bool   inParent;      // accept if is in parent
        int    parent_offset; // if in parent, then the offset should be in
    } widget_state;
    bool inline sizeIsOutlier(const QPoint &new_position, const QWidget *parent) {
        return (parent->width() - new_position.x() < widget_state.parent_offset ||
                parent->height() - new_position.y() < widget_state.parent_offset);
    }

    bool inline positionIsOutlier(const QPoint &new_position) {
        return (new_position.x() + holding_widget->width() < widget_state.parent_offset ||
                new_position.y() + holding_widget->height() < widget_state.parent_offset);
    }

    // acceptable buttons
    QSet<Qt::MouseButton> accept_buttons;

    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);
};

#endif // CCMOVABLEWIDGET_H
