#include <QMouseEvent>
#include "CCMovableWidget.h"

CCMovableWidget::CCMovableWidget(QWidget *parent) : QWidget{ parent } {
    holding_widget             = nullptr;
    widget_state.pressed       = false;
    widget_state.inParent      = true;
    widget_state.parent_offset = 20;
    accept_buttons.insert(Qt::LeftButton);
    setMouseTracking(true);
}

bool CCMovableWidget::eventFilter(QObject *watched, QEvent *event) {
    if (!holding_widget || watched != holding_widget) {
        return false;
    }

    QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(event);
    if (!mouseEvent) {
        return false;
    }

    // here we handle the mouse events
    // this will promise the future extensions
    switch (event->type()) {
        case QEvent::MouseButtonPress:
            handling_mousePressEvent(mouseEvent);
            break;
        case QEvent::MouseButtonRelease:
            handling_mouseReleaseEvent(mouseEvent);
            break;
        case QEvent::MouseMove:
            handling_mouseMoveEvent(mouseEvent);
            break;
        default:
            break;
    }

    // back the default behavior
    return QObject::eventFilter(watched, event);
}

// widget is pressed by the mouse, so this means we shell start our moving
void CCMovableWidget::handling_mousePressEvent(QMouseEvent *event) {
    qDebug() << "Mouse pressed";
    if (!holding_widget)
        return; // no widget to hold, reject process
    if (accept_buttons.size() > 0 && !accept_buttons.contains(event->button()))
        return;                            // the button is not acceptable, reject process
    widget_state.lastPoint = event->pos(); // memorize the last point
    widget_state.pressed   = true;
}

void CCMovableWidget::handling_mouseReleaseEvent(QMouseEvent *event) {
    qDebug() << "Mouse released";
    if (!holding_widget)
        return; // no widget to hold, reject process
    widget_state.pressed = false;
}

void CCMovableWidget::handling_mouseMoveEvent(QMouseEvent *event) {
    qDebug() << "Mouse moved";
    if (!holding_widget)
        return; // no widget to hold, reject process
    if (!widget_state.pressed)
        return; // the widget is not pressed, reject process

    // calculate the offset
    int offsetX = event->pos().x() - widget_state.lastPoint.x();
    int offsetY = event->pos().y() - widget_state.lastPoint.y();

    // calculate the new position
    int x = holding_widget->x() + offsetX;
    int y = holding_widget->y() + offsetY;

    // check if the widget should be in the parent
    if (widget_state.inParent) {
        QWidget *w = dynamic_cast<QWidget *>(holding_widget->parent());
        if (w && (sizeIsOutlier(QPoint(x, y), w) || positionIsOutlier(QPoint(x, y)))) {
            return;
        }

        // move the widget
        holding_widget->move(x, y);
    }
}
