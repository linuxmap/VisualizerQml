#pragma once
#include <QQuickPaintedItem>
#include <QVector>
#include <QPointF>
#include <QLineF>
#include <QPen>
#include "common.h"

#define DRAW_WITH_QPATH   
//#define DRAW_IN_CACHE    
class ElementGroup
{
public:
	ElementGroup()
	{
	}

	ElementGroup(const QPen &pen)
		: m_pen(pen)
	{
	}

	ElementGroup(const ElementGroup &e)
	{
		m_lines = e.m_lines;
		m_pen = e.m_pen;
	}

	ElementGroup & operator=(const ElementGroup &e)
	{
		if (this != &e)
		{
			m_lines = e.m_lines;
			m_pen = e.m_pen;
		}
		return *this;
	}

	~ElementGroup()
	{
	}
#ifdef DRAW_WITH_QPATH
	QVector<QPointF> m_lines;
#else
	QVector<QLineF> m_lines;
#endif
	QPen m_pen;
};

class PaintedItem : public QQuickPaintedItem
{
	Q_OBJECT
		Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)
		Q_PROPERTY(int penWidth READ penWidth WRITE setPenWidth)
		Q_PROPERTY(QColor penColor READ penColor WRITE setPenColor)
		Q_PROPERTY(qreal paintScale READ paintScale WRITE setPaintScale)

public:
	PaintedItem(QQuickItem *parent = 0);
	~PaintedItem();

	bool isEnabled() const { return m_bEnabled; }
	void setEnabled(bool enabled) { m_bEnabled = enabled; }

	int penWidth() const { return m_penWidth; }
	void setPenWidth(int width) { m_penWidth = width; m_pen.setWidth(m_penWidth); }

	QColor penColor() const { return m_pen.color(); }
	void setPenColor(QColor color) { m_pen.setColor(color); }

	qreal paintScale() const { return m_scale; }
	void setPaintScale(qreal scale) 
	{
		m_scale = scale;
		m_pen.setWidthF(m_penWidth*scale);
		hhtdebug("pen width:%d",m_pen.width());
		update();
	}

	Q_INVOKABLE void clear();
	Q_INVOKABLE void undo();

	void paint(QPainter *painter);

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void purgePaintElements();
	void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);
	bool event(QEvent *e) override;

protected:
	QPointF m_lastPoint;
	QVector<ElementGroup*> m_elements;
	ElementGroup * m_element; // the Current ElementGroup
	bool m_bEnabled;
	bool m_bPressed;
	bool m_bMoved;
	QPen m_pen; // the Current Pen
	qreal m_scale;
	int m_penWidth;
};

