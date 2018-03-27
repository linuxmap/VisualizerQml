#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QDebug>
#include "painteditem.hpp"

PaintedItem::PaintedItem(QQuickItem *parent)
	: QQuickPaintedItem(parent)
	, m_element(0)
	, m_bEnabled(true)
	, m_bPressed(false)
	, m_bMoved(false)
	, m_pen(Qt::red)
	, m_scale(1)
	, m_penWidth(5)
{
	setAcceptedMouseButtons(Qt::AllButtons);
	setFlag(ItemAcceptsInputMethod, true);
	m_pen.setCapStyle(Qt::RoundCap);
	m_pen.setJoinStyle(Qt::MiterJoin);
	m_pen.setCosmetic(true);
	m_pen.setWidth(m_penWidth);
	setRenderTarget(FramebufferObject);
	//setMipmap(true);
	//setOpaquePainting(true);
	//setPerformanceHint(FastFBOResizing,true);
}

PaintedItem::~PaintedItem()
{
	purgePaintElements();
}

void PaintedItem::clear()
{
	purgePaintElements();
	update();
}

void PaintedItem::undo()
{
	if (m_elements.size())
	{
		delete m_elements.takeLast();
		update();
	}
}

void PaintedItem::paint(QPainter *painter)
{
	painter->setRenderHint(QPainter::Antialiasing);
	QPointF center(width() / qreal(2), height() / qreal(2));
	painter->translate(center);
	painter->scale(m_scale,m_scale);
	//hhtdebug("Paint:scale:%f",m_scale);
	//m_pen.setWidth(m_penWidth);
	painter->translate(-center);

	int size = m_elements.size();
	ElementGroup *element;
	if (!size)return;
#ifdef DRAW_IN_CACHE
#else
	for (int i = 0; i < size; ++i)
	{
		element = m_elements.at(i);
		QPen pen(element->m_pen);
		pen.setWidthF(pen.width()*m_scale);
		painter->setPen(pen);
#ifdef DRAW_WITH_QPATH
		QPainterPath path;
		path.moveTo(element->m_lines.first());
		int pointCnt = element->m_lines.size();
		for (int j = 0;j < pointCnt;j++)
		{
				path.lineTo(element->m_lines.at(j));

		}
		//path.lineTo(element->m_lines.last());
		painter->drawPath(path);
#else
		painter->drawLines(element->m_lines);
#endif


	}
#endif
}

void PaintedItem::mousePressEvent(QMouseEvent *event)
{
	m_bMoved = false;
	if (!m_bEnabled || !(event->button() & acceptedMouseButtons()))
	{
		QQuickPaintedItem::mousePressEvent(event);
	}
	else
	{
		//qDebug() << "mouse pressed";
		m_bPressed = true;
		m_element = new ElementGroup(m_pen);
		m_elements.append(m_element);
		m_lastPoint = event->localPos();
		event->setAccepted(true);
	}
}

void PaintedItem::mouseMoveEvent(QMouseEvent *event)
{
	if (!m_bEnabled || !m_bPressed || !m_element)
	{
		QQuickPaintedItem::mousePressEvent(event);
	}
	else
	{
#ifdef DRAW_WITH_QPATH
		m_element->m_lines.append(event->localPos());
#else
		m_element->m_lines.append(QLineF(m_lastPoint, event->localPos()));
#endif
		m_lastPoint = event->localPos();
		update();
		//event->setAccepted(false);

	}
}

void PaintedItem::mouseReleaseEvent(QMouseEvent *event)
{
	if (!m_element || !m_bEnabled || !(event->button() & acceptedMouseButtons()))
	{
		QQuickPaintedItem::mousePressEvent(event);
	}
	else
	{
		m_bPressed = false;
		m_bMoved = false;
#ifdef DRAW_WITH_QPATH
		m_element->m_lines.append(event->localPos());
#else
		if(m_lastPoint==event->localPos())
			m_element->m_lines.append(QLineF(m_lastPoint, QPointF(m_lastPoint.x()+1, m_lastPoint.y()+1)));
		else
			m_element->m_lines.append(QLineF(m_lastPoint, event->localPos()));
#endif
		update();
		//event->setAccepted(false);

	}
}

void PaintedItem::purgePaintElements()
{
	int size = m_elements.size();
	if (size > 0)
	{
		for (int i = 0; i < size; ++i)
		{
			delete m_elements.at(i);
		}
		m_elements.clear();
	}
	m_element = 0;
}

void PaintedItem::geometryChanged(const QRectF & newGeometry, const QRectF & oldGeometry)
{
	//if (newGeometry.size() == QSizeF(0, 0) || oldGeometry.size() == QSizeF(0, 0))return;
	//qreal scale = newGeometry.width() / oldGeometry.width();
	//scale = scale*this->scale();
	//setTransformOrigin(QQuickItem::TopLeft);

	//setScale(scale);
	//setTransformOrigin(QQuickItem::Center);

	
	
	QQuickPaintedItem::geometryChanged(newGeometry,oldGeometry);
}

bool PaintedItem::event(QEvent * e)
{
	//qDebug() << "PaintedItem: " << e;
	//switch (e->type())
	//{

	//}
	return QQuickPaintedItem::event(e);
}
