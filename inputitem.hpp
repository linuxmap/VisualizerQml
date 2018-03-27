#pragma once
#include <QQuickItem>
#include "common.h"

class InputItem : public QQuickItem {
	Q_OBJECT
	Q_PROPERTY(QQuickItem *target READ target WRITE setTarget)
public:
	InputItem(QQuickItem * parent = Q_NULLPTR);
	~InputItem();
	QQuickItem * target(void) const;
	void setTarget(QQuickItem * target);
signals:
	void targetScaleChanged(qreal scale);
	void targetRotChanged(qreal angle);
	void targetXChanged(qreal x);
	void targetYChanged(qreal y);
	void rightClicked(qreal x,qreal y);
protected:
	void mousePressEvent(QMouseEvent *e) override;
	void mouseMoveEvent(QMouseEvent *e)override;
	void mouseReleaseEvent(QMouseEvent *e)override;

	void wheelEvent(QWheelEvent *e)override;
	void touchEvent(QTouchEvent *e)override;
	bool event(QEvent *e)override;
private:
	const QPointF pointsCenter(const QList<QTouchEvent::TouchPoint> & points);
	const qreal   pointsScale(const QPointF &center, const QPointF &lastCenter,const QList<QTouchEvent::TouchPoint> & points);
	const qreal   pointsAngle(const QList<QTouchEvent::TouchPoint> & points);
	void targetMove(const QPointF &p);
	void targetMove(const qreal x,const qreal y);

	QQuickItem *m_pTarget;
	bool m_bMousePressed;
	QPointF m_lastMousePointF;


	QPointF m_lastTouchCenter;
	int     m_lastPointsCnt;

};
