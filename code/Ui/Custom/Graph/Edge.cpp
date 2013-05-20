#include "Core/Math/Const.h"
#include "Core/Math/Envelope.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Vector2.h"
#include "Ui/Canvas.h"
#include "Ui/Event.h"
#include "Ui/Custom/Graph/Edge.h"
#include "Ui/Custom/Graph/Pin.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/PaintSettings.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int c_sourcePinOffset = 10;
const int c_destPinOffset = 12;

void calculateLinearSpline(Point s1, Point d1, std::vector< Point >& outSpline)
{
	Point s = s1, d = d1;

	s.x += c_sourcePinOffset;
	d.x -= c_destPinOffset;

	Point r(d.x - s.x, d.y - s.y);
	Point ar(traktor::abs(r.x), traktor::abs(r.y));
	Point c((s.x + d.x) / 2, (s.y + d.y) / 2);

	Point m1, m2;

	if (s.x <= d.x)
	{
		if (ar.x >= ar.y)
		{
			m1 = Point(c.x - ar.y / 2, s.y);
			m2 = Point(c.x + ar.y / 2, d.y);
		}
		else
		{
			if (s.y < d.y)
			{
				m1 = Point(s.x, c.y - r.x / 2);
				m2 = Point(d.x, c.y + r.x / 2);
			}
			else
			{
				m1 = Point(s.x, c.y + r.x / 2);
				m2 = Point(d.x, c.y - r.x / 2);
			}
		}
	}
	else
	{
		if (ar.x >= ar.y)
		{
			m1 = Point(s.x, c.y);
			m2 = Point(d.x, c.y);
		}
		else
		{
			if (s.y < d.y)
			{
				m1 = Point(s.x, c.y + r.x / 2);
				m2 = Point(d.x, c.y - r.x / 2);
			}
			else
			{
				m1 = Point(s.x, c.y - r.x / 2);
				m2 = Point(d.x, c.y + r.x / 2);
			}
		}
	}

	outSpline.resize(6);
	outSpline[0] = s1;
	outSpline[1] = s;
	outSpline[2] = m1;
	outSpline[3] = m2;
	outSpline[4] = d;
	outSpline[5] = d1;
}

void calculateSmoothSpline(Point s1, Point d1, std::vector< Point >& outSpline)
{
	Envelope< Vector2, HermiteEvaluator< Vector2 > > envelope;

	int dx = d1.x - s1.x;
	int dy = d1.y - s1.y;

	envelope.addKey(0.0f, Vector2(float(s1.x), float(s1.y)));
	envelope.addKey(0.2f, Vector2(s1.x + dx * 0.25f, s1.y + dy * 0.1f));
	envelope.addKey(0.5f, Vector2((s1.x + d1.x) / 2.0f, (s1.y + d1.y) / 2.0f));
	envelope.addKey(0.8f, Vector2(d1.x - dx * 0.25f, d1.y - dy * 0.1f));
	envelope.addKey(1.0f, Vector2(float(d1.x), float(d1.y)));

	outSpline.resize(30);
	for (int i = 0; i < 30; ++i)
	{
		Vector2 p = envelope(i / 29.0f);
		outSpline[i] = Point(int(p.x), int(p.y));
	}
}

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.Edge", Edge, Object)

Edge::Edge(Pin* source, Pin* destination)
:	m_source(source)
,	m_destination(destination)
,	m_selected(false)
{
}

void Edge::setSourcePin(Pin* source)
{
	m_source = source;
}

Ref< Pin > Edge::getSourcePin() const
{
	return m_source;
}

void Edge::setDestinationPin(Pin* destination)
{
	m_destination = destination;
}

Ref< Pin > Edge::getDestinationPin() const
{
	return m_destination;
}

void Edge::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& Edge::getText() const
{
	return m_text;
}

void Edge::setSelected(bool selected)
{
	m_selected = selected;
}

bool Edge::isSelected() const
{
	return m_selected;
}

bool Edge::hit(const PaintSettings* paintSettings, const Point& p) const
{
	Vector2 P(float(p.x), float(p.y));

	if (paintSettings->getSmoothSpline())
		calculateSmoothSpline(m_source->getPosition(), m_destination->getPosition(), m_spline);
	else
		calculateLinearSpline(m_source->getPosition(), m_destination->getPosition(), m_spline);

	for (int i = 1; i < int(m_spline.size() - 2); ++i)
	{
		const Point& s = m_spline[i];
		const Point& d = m_spline[i + 1];

		Vector2 v(float(d.x - s.x), float(d.y - s.y));
		if (v.length() <= FUZZY_EPSILON)
			continue;
		
		Vector2 V = v.perpendicular();
		Vector2 R = Vector2(float(s.x), float(s.y)) - P;

		float D = traktor::abs(dot(V, R) / V.length());
		if (D > 4.0f)
			continue;

		Vector2 Pr = P + V * D;
		float Dr = dot(Pr - Vector2(float(s.x), float(s.y)), v) / (v.length() * v.length());
		if (Dr < 0.0f || Dr > 1.0f)
			continue;
		
		return true;
	}

	return false;
}

void Edge::paint(const PaintSettings* paintSettings, Canvas* canvas, const Size& offset) const
{
	if (!m_source || !m_destination)
		return;

	if (m_selected)
	{
		canvas->setForeground(paintSettings->getEdgeSelected());
		canvas->setBackground(paintSettings->getEdgeSelected());
		canvas->setPenThickness(3);
	}
	else
	{
		canvas->setForeground(paintSettings->getEdge());
		canvas->setBackground(paintSettings->getEdge());
	}

	Point s = m_source->getPosition() + offset;
	Point d = m_destination->getPosition() + offset;

	if (paintSettings->getSmoothSpline())
		calculateSmoothSpline(s, d, m_spline);
	else
		calculateLinearSpline(s, d, m_spline);

	canvas->drawLines(m_spline);

	if (m_selected)
		canvas->setPenThickness(1);

	Point at = m_destination->getPosition() + offset;
	Point arrow[] =
	{
		Point(at.x - 8, at.y - 5),
		at,
		Point(at.x - 8, at.y + 5)
	};
	canvas->fillPolygon(arrow, 3);
}

		}
	}
}
