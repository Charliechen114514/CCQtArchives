#include "CCArcProgressWidget.h"
#include <QPropertyAnimation>

CCArcProgressWidget::CCArcProgressWidget(QWidget* parent)
	: QWidget { parent } {
	setupAnimation();
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

void CCArcProgressWidget::paintEvent(QPaintEvent* event [[maybe_unused]]) {
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);

	QRectF baseRect = rect();

	double side = qMin(baseRect.width(), baseRect.height());

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

void CCArcProgressWidget::setupAnimation() {
	animation = new QPropertyAnimation(this, "displayValue");
	animation->setDuration(DURATION);
	animation->setEasingCurve(QEasingCurve::OutCubic);
}

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
