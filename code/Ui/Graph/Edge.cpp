/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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

namespace traktor::ui
{
	namespace
	{

struct Dim
{
	int32_t sourcePinOffset = 10;
	int32_t destPinOffset = 16;
	int32_t sourceLabelOffset = 10;

	Dim(const Widget* widget)
	{
		sourcePinOffset = widget->pixel(Unit(sourcePinOffset));
		destPinOffset = widget->pixel(Unit(destPinOffset));
		sourceLabelOffset = widget->pixel(Unit(sourceLabelOffset));
	}
};

void calculateLinearSpline(const GraphControl* graph, Point s1, Point d1, AlignedVector< Point >& outSpline)
{
	const Dim dim(graph);
	Point s = s1, d = d1;

	s.x += dim.sourcePinOffset;
	d.x -= dim.destPinOffset;

	const Point r(d.x - s.x, d.y - s.y);
	const Point ar(traktor::abs(r.x), traktor::abs(r.y));
	const Point c((s.x + d.x) / 2, (s.y + d.y) / 2);

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
	outSpline[5] = Point(d1.x - graph->pixel(6_ut), d1.y);
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

void Edge::setThickness(Unit thickness)
{
	m_thickness = thickness;
}

Unit Edge::getThickness() const
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

bool Edge::hit(const GraphControl* graph, const UnitPoint& p) const
{
	const Point pt = graph->pixel(p);
	const Vector2 P(float(pt.x), float(pt.y));

	calculateLinearSpline(
		graph,
		graph->pixel(m_source->getPosition()),
		graph->pixel(m_destination->getPosition()),
		m_spline
	);

	const float c_hitWidth = (float)graph->pixel(8_ut);
	for (int32_t i = 1; i < (int32_t)(m_spline.size() - 2); ++i)
	{
		const Point& s = m_spline[i];
		const Point& d = m_spline[i + 1];

		const Vector2 v(float(d.x - s.x), float(d.y - s.y));
		if (v.length() <= FUZZY_EPSILON)
			continue;

		const Vector2 V = v.perpendicular();
		const Vector2 R = Vector2(float(s.x), float(s.y)) - P;

		const float D = traktor::abs(dot(V, R) / V.length());
		if (D > c_hitWidth)
			continue;

		const Vector2 Pr = P + V * D;
		const float Dr = dot(Pr - Vector2(float(s.x), float(s.y)), v) / (v.length() * v.length());
		if (Dr < 0.0f || Dr > 1.0f)
			continue;

		return true;
	}

	return false;
}

void Edge::paint(GraphControl* graph, GraphCanvas* canvas, const Size& offset, IBitmap* imageLabel, bool hot) const
{
	if (!m_source || !m_destination)
		return;

	const StyleSheet* ss = graph->getStyleSheet();
	const PaintSettings& settings = canvas->getPaintSettings();
	const Dim dim(graph);

	Color4ub color;
	if (hot)
		color = ss->getColor(this, L"color-hover");
	else if (m_selected)
		color = ss->getColor(this, L"color-selected");
	else
		color = ss->getColor(this, L"color");

	canvas->setForeground(color);
	canvas->setBackground(color);

	const Point s = graph->pixel(m_source->getPosition()) + offset;
	const Point d = graph->pixel(m_destination->getPosition()) + offset;

	calculateLinearSpline(graph, s, d, m_spline);

#if defined(_DEBUG)
	canvas->setBackground(Color4ub(255, 255, 255, 255));
	for (const auto& p : m_spline)
	{
		canvas->fillRect(
			Rect(
				Point(p.x - 1, p.y - 1),
				Size(2, 2)
			)
		);
	}
	canvas->setBackground(color);
#endif

	canvas->drawLines(m_spline, graph->pixel(hot ? 4_ut : m_thickness));

	const Point at = graph->pixel(m_destination->getPosition()) + offset;
	const Point arrow[] =
	{
		Point(at.x - graph->pixel(10_ut), at.y - graph->pixel(5_ut)),
		Point(at.x , at.y),
		Point(at.x - graph->pixel(10_ut), at.y + graph->pixel(5_ut))
	};
	canvas->fillPolygon(arrow, 3);

	if (imageLabel && !m_text.empty())
	{
		canvas->setForeground(ss->getColor(this, L"color-label"));
		canvas->setFont(settings.getFontLabel());

		const auto sz = imageLabel->getSize(graph);
		const Size szLabel = canvas->getTextExtent(m_text);

		const Point lpt(
			s.x + dim.sourceLabelOffset,
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
