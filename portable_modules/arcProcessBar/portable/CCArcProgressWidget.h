#ifndef CCARCPROGRESSWIDGET_H
#define CCARCPROGRESSWIDGET_H

#include <QPainter>
#include <QWidget>
class QPropertyAnimation;

class CCArcProgressWidget : public QWidget {
	Q_OBJECT
	Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
	Q_PROPERTY(int maxValue READ maxValue WRITE setMaxValue NOTIFY maxValueChanged)
	Q_PROPERTY(int displayValue READ displayValue WRITE setDisplayValue NOTIFY displayValueChanged)

	/**
	 * @brief DURATION means the animation process durations
	 */
	static constexpr const unsigned int DURATION = 500;
	/**
	 * @brief ARC_WIDTH the width of the arc
	 */
	static constexpr const unsigned short ARC_WIDTH = 50;
	/**
	 * @brief DEFAULT_VALUE means the default settings value,
	 *
	 */
	static constexpr const unsigned short DEFAULT_VALUE = 0;
	/**
	 * @brief DEFAULT_MAX means the max value
	 */
	static constexpr const unsigned short DEFAULT_MAX = 100;
	/**
	 * @brief DEF_TEXT_COLOR the text color display settings
	 */
	static constexpr const QColor DEF_TEXT_COLOR = QColor(50, 50, 50);
	/**
	 * @brief DEF_BKCOLOR the background color of the arc
	 */
	static constexpr const QColor DEF_BKCOLOR = QColor(255, 255, 255);
	/**
	 * @brief DEF_ARC_COLOR
	 */
	static constexpr const QColor DEF_ARC_COLOR = QColor(65, 105, 225);

public:
	explicit CCArcProgressWidget(QWidget* parent = nullptr);
	/**
	 * @brief value the value getter
	 * @return the current value
	 */
	inline int value() const { return progress_value; }
	/**
	 * @brief maxValue the max value wating for set
	 * @return maxValue the progressbar set
	 */
	inline int maxValue() const { return progress_max_value; }

	/**
	 * @brief setValue ayncs value sets
	 * @param val waiting for the set, invoke this for the details
	 */
	void setValue(int val);

	/**
	 * @brief setMaxValue max value settings
	 * @param max
	 */
	inline void setMaxValue(int max) {
		if (max > 0 && max != progress_max_value) {
			progress_max_value = max;
			update();
		}
	}

	/**
	 * @brief Returns the color of the progress arc.
	 * @return The color of the progress arc.
	 */
	inline QColor progressArcColor() const {
		return progress_arc_color;
	}

	/**
	 * @brief Sets the color of the progress arc.
	 * @param color The new color for the progress arc.
	 */
	inline void setProgressArcColor(const QColor& color) {
		if (progress_arc_color == color) {
			return;
		}
		progress_arc_color = color;
		update();
	}

	/**
	 * @brief Returns the color of the progress background arc.
	 * @return The color of the progress background arc.
	 */
	inline QColor progressBackgroundColor() const {
		return progress_backgroundColor;
	}

	/**
	 * @brief Sets the color of the progress background arc.
	 * @param color The new color for the progress background arc.
	 */
	inline void setProgressBackgroundColor(const QColor& color) {
		if (progress_backgroundColor == color) {
			return;
		}
		progress_backgroundColor = color;
		update();
	}

	/**
	 * @brief Returns the color of the progress text.
	 * @return The color of the progress text.
	 */
	inline QColor progressTextColor() const {
		return progress_textColor;
	}

	/**
	 * @brief Sets the color of the progress text.
	 * @param color The new color for the progress text.
	 */
	inline void setProgressTextColor(const QColor& color) {
		if (progress_textColor == color) {
			return;
		}
		progress_textColor = color;
		update();
	}

	/**
	 * @brief Signal emitted when the progress value changes.
	 * @param newValue The new progress value.
	 */
signals:
	void valueChanged(int newValue);

	/**
	 * @brief Signal emitted when the maximum progress value changes.
	 * @param newMaxValue The new maximum value.
	 */
	void maxValueChanged(int newMaxValue);

	/**
	 * @brief Signal emitted when the current displayed value changes.
	 * @param currentDisplayValue The new displayed value.
	 */
	void displayValueChanged(int currentDisplayValue);

protected:
	/**
	 * @brief Handles the paint event to draw the progress indicator.
	 * @param event The paint event.
	 */
	void paintEvent(QPaintEvent* event) override;

private:
	/**
	 * @brief Sets up the animation for the progress indicator.
	 */
	void setupAnimation();

	/**
	 * @brief Draws the background arc of the progress indicator.
	 * @param painter The painter object to use.
	 * @param arcRect The rectangle defining the arc area.
	 */
	void drawBackgroundArc(QPainter& painter, const QRectF& arcRect);

	/**
	 * @brief Draws the progress arc of the progress indicator.
	 * @param painter The painter object to use.
	 * @param arcRect The rectangle defining the arc area.
	 * @param angle The angle representing the current progress.
	 */
	void drawProgressArc(QPainter& painter, const QRectF& arcRect, double angle);

	/**
	 * @brief Draws the text of the progress indicator.
	 * @param painter The painter object to use.
	 * @param center The center point of the arc.
	 * @param radius The radius of the arc.
	 */
	void drawText(QPainter& painter, const QPointF& center, double radius);

private:
	int progress_value { DEFAULT_VALUE };
	int progress_display_value { DEFAULT_VALUE };
	int progress_max_value { DEFAULT_MAX };
	int progress_minAngle { -360 };
	int progress_startAngle { 90 };
	QColor progress_arc_color { DEF_ARC_COLOR };
	QColor progress_backgroundColor { DEF_BKCOLOR };
	QColor progress_textColor = { DEF_TEXT_COLOR };
	QPropertyAnimation* animation { nullptr };

	/**
	 * @brief Sets the current displayed progress value.
	 * @param val The new displayed value.
	 */
	void setDisplayValue(int val) {
		if (progress_display_value == val)
			return;
		progress_display_value = val;
		update();
	}

	/**
	 * @brief Returns the current displayed progress value.
	 * @return The displayed progress value.
	 */
	inline int displayValue() const { return progress_display_value; }
};

#endif // CCARCPROGRESSWIDGET_H
