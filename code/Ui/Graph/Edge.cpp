#include "Core/Math/Const.h"
#include "Core/Math/Envelope.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Vector2.h"
#include "Ui/Application.h"
#include "Ui/Event.h"
#include "Ui/IBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Graph/Edge.h"
#include "Ui/Graph/GraphCanvas.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/Pin.h"
#include "Ui/Graph/PaintSettings.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const int32_t c_sourcePinOffset = 10;
const int32_t c_destPinOffset = 16;
const int32_t c_sourceLabelOffset = 10;

void calculateLinearSpline(Point s1, Point d1, AlignedVector< Point >& outSpline)
{
	Point s = s1, d = d1;

	s.x += dpi96(c_sourcePinOffset);
	d.x -= dpi96(c_destPinOffset);

	Point r(d.x - s.x, d.y - s.y);
	Point ar(traktor::abs(r.x), traktor::abs(r.y));
	Point c((s.x + d.x) / 2, (s.y + d.y) / 2);

	Point m1, m2;

	if (s.x <= d.x)
	{
		if (ar.x >= ar.y)
		{
			m1 = Point(c.x - ar.y / 2, s.y);
			m2 = Point(c.x + (ar.y + 1) / 2, d.y);
		}
		else
		{
			if (s.y < d.y)
			{
				m1 = Point(s.x, c.y - r.x / 2);
				m2 = Point(d.x, c.y + (r.x + 1) / 2);
			}
			else
			{
				m1 = Point(s.x, c.y + r.x / 2);
				m2 = Point(d.x, c.y - (r.x + 1) / 2);
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
				m2 = Point(d.x, c.y - (r.x + 1) / 2);
			}
			else
			{
				m1 = Point(s.x, c.y - r.x / 2);
				m2 = Point(d.x, c.y + (r.x + 1) / 2);
			}
		}
	}

	outSpline.resize(6);
	outSpline[0] = Point(s1.x, s1.y);
	outSpline[1] = s;
	outSpline[2] = m1;
	outSpline[3] = m2;
	outSpline[4] = d;
	outSpline[5] = Point(d1.x - dpi96(6), d1.y);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Edge", Edge, Object)

Edge::Edge(Pin* source, Pin* destination)
:	m_source(source)
,	m_destination(destination)
{
}

void Edge::setSourcePin(Pin* source)
{
	m_source = source;
}

Pin* Edge::getSourcePin() const
{
	return m_source;
}

void Edge::setDestinationPin(Pin* destination)
{
	m_destination = destination;
}

Pin* Edge::getDestinationPin() const
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

void Edge::setThickness(int32_t thickness)
{
	m_thickness = thickness;
}

int32_t Edge::getThickness() const
{
	return m_thickness;
}

void Edge::setSelected(bool selected)
{
	m_selected = selected;
}

bool Edge::isSelected() const
{
	return m_selected;
}

bool Edge::hit(const Point& p) const
{
	Vector2 P(float(p.x), float(p.y));

	calculateLinearSpline(m_source->getPosition(), m_destination->getPosition(), m_spline);

	const float c_hitWidth = float(dpi96(4));

	for (int32_t i = 1; i < (int32_t)(m_spline.size() - 2); ++i)
	{
		const Point& s = m_spline[i];
		const Point& d = m_spline[i + 1];

		Vector2 v(float(d.x - s.x), float(d.y - s.y));
		if (v.length() <= FUZZY_EPSILON)
			continue;

		Vector2 V = v.perpendicular();
		Vector2 R = Vector2(float(s.x), float(s.y)) - P;

		float D = traktor::abs(dot(V, R) / V.length());
		if (D > c_hitWidth)
			continue;

		Vector2 Pr = P + V * D;
		float Dr = dot(Pr - Vector2(float(s.x), float(s.y)), v) / (v.length() * v.length());
		if (Dr < 0.0f || Dr > 1.0f)
			continue;

		return true;
	}

	return false;
}

void Edge::paint(GraphControl* graph, GraphCanvas* canvas, const Size& offset, IBitmap* imageLabel) const
{
	if (!m_source || !m_destination)
		return;

	const StyleSheet* ss = graph->getStyleSheet();
	const PaintSettings* settings = canvas->getPaintSettings();

	auto color = ss->getColor(this, m_selected ? L"color-selected" : L"color");
	canvas->setForeground(color);
	canvas->setBackground(color);

	Point s = m_source->getPosition() + offset;
	Point d = m_destination->getPosition() + offset;

	calculateLinearSpline(s, d, m_spline);
	canvas->drawLines(m_spline, dpi96(m_thickness));

	Point at = m_destination->getPosition() + offset;
	Point arrow[] =
	{
		Point(at.x - dpi96(12), at.y - dpi96(5)),
		Point(at.x - dpi96(2) , at.y),
		Point(at.x - dpi96(12), at.y + dpi96(5))
	};
	canvas->fillPolygon(arrow, 3);

	if (imageLabel && !m_text.empty())
	{
		canvas->setForeground(ss->getColor(this, L"color-label"));
		canvas->setFont(settings->getFontLabel());

		const auto sz = imageLabel->getSize();
		Size szLabel = canvas->getTextExtent(m_text);

		Point lpt(
			s.x + dpi96(c_sourceLabelOffset),
			s.y - sz.cy / 2
		);

		canvas->drawBitmap(
			lpt,
			Size(sz.cx / 3, sz.cy),
			Point(0, 0),
			Size(sz.cx / 3, sz.cy),
			imageLabel,
			BlendMode::Alpha
		);

		canvas->drawBitmap(
			lpt + Size(sz.cx / 3, 0),
			Size(szLabel.cx, sz.cy),
			Point(sz.cx / 3, 0),
			Size(sz.cx / 3, sz.cy),
			imageLabel,
			BlendMode::Alpha
		);

		canvas->drawBitmap(
			lpt + Size(sz.cx / 3 + szLabel.cx, 0),
			Size(sz.cx / 3, sz.cy),
			Point((sz.cx * 2) / 3, 0),
			Size(sz.cx / 3, sz.cy),
			imageLabel,
			BlendMode::Alpha
		);

		canvas->drawText(
			Rect(
				lpt + Size(sz.cx / 3, 0),
				Size(szLabel.cx, sz.cy)
			),
			m_text,
			AnCenter,
			AnCenter
		);
	}
}

	}
}
