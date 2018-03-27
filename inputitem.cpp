#include "inputitem.hpp"
#include <QLineF> 

InputItem::InputItem(QQuickItem * parent) :
	QQuickItem(parent)
	, m_pTarget(NULL)
	, m_lastTouchCenter(0,0)
{
	//setAcceptHoverEvents(true);
	setAcceptedMouseButtons(Qt::AllButtons);
	setFiltersChildMouseEvents(true);
	//setFlag(ItemAcceptsInputMethod, true);
	//setKeepTouchGrab(true);

	
}

InputItem::~InputItem() {

}

QQuickItem * InputItem::target(void) const
{
	return m_pTarget;
}

void InputItem::setTarget(QQuickItem * target)
{
	m_pTarget = target;
}

void InputItem::mousePressEvent(QMouseEvent * e)
{
	if (!m_pTarget)
	{
		return QQuickItem::mouseMoveEvent(e);
	}
	if (e->source() == Qt::MouseEventNotSynthesized)
	{
		//hhtdebug("mouse press");
		m_bMousePressed = true;
		m_lastMousePointF = e->localPos();
	}
}

void InputItem::mouseMoveEvent(QMouseEvent * e)
{
	if (!m_pTarget)
	{
		e->setAccepted(false);
		return QQuickItem::mouseMoveEvent(e);
	}

	if (e->source() == Qt::MouseEventNotSynthesized)
	{
		if (m_bMousePressed)
		{

			qreal xdiff = e->localPos().x() - m_lastMousePointF.x();
			qreal ydiff = e->localPos().y() - m_lastMousePointF.y();
			targetMove(xdiff, ydiff);
			m_lastMousePointF = e->localPos();
			e->accept();

		}
	}
}

void InputItem::mouseReleaseEvent(QMouseEvent * e)
{
	if (!m_pTarget)
	{
		return QQuickItem::mouseMoveEvent(e);
	}
	if (e->source() == Qt::MouseEventNotSynthesized)
	{
		m_bMousePressed = false;
	}
	if (e->button() == Qt::RightButton)
		emit rightClicked(e->localPos().x(), e->localPos().y());
}

void InputItem::wheelEvent(QWheelEvent * e)
{
	if (!m_pTarget)
	{
		return QQuickItem::wheelEvent(e);
	}
	QPoint numPixels = e->pixelDelta();
	QPoint numDegrees = e->angleDelta() / 8;

	if (!numPixels.isNull()) {

	}
	else if (!numDegrees.isNull()) {
		QPoint numSteps = numDegrees / 15;
		qreal scale = m_pTarget->scale();
		if (numSteps.y() > 0)
		{
			scale += scale / 10;
		}
		else if (numSteps.y() < 0)
		{
			scale -= scale / 10;
			if (scale < 0.1)scale = 0.1;
		}
		m_pTarget->setScale(scale);
		emit targetScaleChanged(scale);
	}

	e->accept();

}

void InputItem::touchEvent(QTouchEvent * e)
{
	switch (e->type())
	{
	case QEvent::TouchBegin:
		{
			QPointF center = pointsCenter(e->touchPoints());
			m_lastTouchCenter = center;
			m_lastPointsCnt = e->touchPoints().count();

		}
		break;
	case QEvent::TouchUpdate:
		{
			QPointF center = pointsCenter(e->touchPoints());
			int pointsCnt = e->touchPoints().count();
			if (pointsCnt != m_lastPointsCnt) //some touch point released
			{
				m_lastPointsCnt = pointsCnt;
				m_lastTouchCenter = center;
				e->accept();
				return QQuickItem::touchEvent(e);
			}
			QPointF centerMove = center - m_lastTouchCenter;
			targetMove(centerMove);

			if (pointsCnt > 1)
			{
				qreal scale=pointsScale(center, m_lastTouchCenter, e->touchPoints());
				scale = scale*m_pTarget->scale();
				m_pTarget->setScale(scale);

				qreal angle = pointsAngle(e->touchPoints());
				//hhtdebug("Angle:%f",angle);
				angle += m_pTarget->rotation();
				m_pTarget->setRotation(angle);
			}




			m_lastTouchCenter = center;
		}
		break;
	case QEvent::TouchEnd:
		break;
	case QEvent::TouchCancel:

		break;
		default:break;

	}
	e->accept();
}

const QPointF InputItem::pointsCenter(const QList<QTouchEvent::TouchPoint> & points)
{
	QPointF center;
	QPointF sum(0, 0);
	for (int i = 0;i < points.count();i++)
	{
		sum += points.at(i).pos();
	}
	center = sum / points.size();
	return center;
}


const qreal InputItem::pointsScale(const QPointF &center, const QPointF &lastCenter,const QList<QTouchEvent::TouchPoint> & points)
{
	int pointsCnt = points.count();
	qreal scale=0;
	if (pointsCnt < 2)return scale;
	qreal distSum=0;
	qreal lastDistSum = 0;

	for (int i = 0;i < pointsCnt;i++)
	{
		qreal dist = QLineF(points.at(i).pos(), center).length();
		distSum += dist;

		qreal lastDist = QLineF(points.at(i).lastPos(), lastCenter).length();
		lastDistSum += lastDist;

	}
	scale = distSum / lastDistSum;
	return scale;
}


const qreal InputItem::pointsAngle(const QList<QTouchEvent::TouchPoint> & points)
{
	qreal angle = 0;
	if (points.count() < 2)return angle;

	QPointF point1 = points.at(0).pos();
	QPointF point2 = points.at(1).pos();
	QLineF  line(point1, point2);

	QPointF lastPoint1 = points.at(0).lastPos();
	QPointF lastPoint2 = points.at(1).lastPos();
	QLineF  lastLine(lastPoint1, lastPoint2);

	angle = line.angleTo(lastLine);
	return angle;
}

void InputItem::targetMove(const QPointF &p)
{
	targetMove(p.x(),p.y());
}

void InputItem::targetMove(const qreal x, const qreal y)
{
	if (!m_pTarget)return;
	qreal xnew = m_pTarget->x() + x;
	qreal ynew = m_pTarget->y() + y;
	m_pTarget->setX(xnew);
	m_pTarget->setY(ynew);
}
#if 1

bool InputItem::event(QEvent * e)
{
	//qDebug() << e->type();
	//switch (e->type())
	//{
	//	default:break;
	//}

	return QQuickItem::event(e);
}
#endif


