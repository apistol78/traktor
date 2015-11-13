#pragma optimize( "", off )

#include <algorithm>
#include <cfloat>
#include "Core/Log/Log.h"
#include "Core/Math/Bezier2nd.h"
#include "Core/Math/Format.h"
#include "Core/Math/Line2.h"
#include "Flash/Polygon.h"
#include "Flash/Editor/FlashPathControl.h"
#include "Ui/Application.h"
#include "Ui/StyleSheet.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

class Painter
{
public:
	Painter(ui::Canvas& canvas, const ui::Rect& rc, const Vector2& viewOffset, float viewScale)
	:	m_canvas(canvas)
	,	m_rc(rc)
	,	m_viewOffset(viewOffset)
	,	m_viewScale(viewScale)
	,	m_tail(ui::scaleBySystemDPI(4))
	{
	}

	void drawHLine(float y, const Color4ub& cl)
	{
		float cy = project(Vector2(0.0f, y)).y;
		m_canvas.setForeground(cl);
		m_canvas.drawLine(
			ui::Point(m_rc.left, cy),
			ui::Point(m_rc.right, cy)
		);
	}

	void drawLine(const Vector2& p0, const Vector2& p1, const Color4ub& cl)
	{
		Vector2 c0 = project(p0);
		Vector2 c1 = project(p1);

		Vector2 d = (c1 - c0);
		
		float ln = d.length();
		if (ln > m_tail * 2.0f)
		{
			d /= ln;

			Color4ub tcl(255, 0, 0, cl.a);

			m_canvas.setForeground(tcl);
			m_canvas.drawLine(
				toPoint(c0),
				toPoint(c0 + d * m_tail)
			);
			m_canvas.drawLine(
				toPoint(c1 - d * m_tail),
				toPoint(c1)
			);

			m_canvas.setForeground(cl);
			m_canvas.drawLine(
				toPoint(c0 + d * m_tail),
				toPoint(c1 - d * m_tail)
			);
		}
		else
		{
			m_canvas.setForeground(cl);
			m_canvas.drawLine(
				toPoint(c0),
				toPoint(c1)
			);
		}
	}

	void drawCurve(const Vector2& p0, const Vector2& pc, const Vector2& p1, const Color4ub& cl)
	{
		Color4ub tcl(255, 0, 0, cl.a);

		Bezier2nd b(p0, pc, p1);
		for (int32_t t = 0; t < 10; ++t)
		{
			float f0 = float(t / 10.0f);
			float f1 = float((t + 1) / 10.0f);

			Vector2 c0 = b.evaluate(f0);
			Vector2 c1 = b.evaluate(f1);

			m_canvas.setForeground((t == 0 || t == 9) ? tcl : cl);
			m_canvas.drawLine(
				toPoint(project(c0)),
				toPoint(project(c1))
			);
		}
	}

	void drawTriangle(const Triangle& t)
	{
		ui::Point pnts[] =
		{
			toPoint(project(t.v[0])),
			toPoint(project(t.v[1])),
			toPoint(project(t.v[2]))
		};
		m_canvas.setBackground(Color4ub(255, 255, 0, 40));
		m_canvas.fillPolygon(pnts, 3);
	}

	Vector2 clientToView(const ui::Point& pt) const
	{
		Vector2 p = (Vector2(pt.x, pt.y) - Vector2(m_rc.left, m_rc.top)) / Vector2(m_rc.getWidth(), m_rc.getHeight());
		return (p * m_viewScale) + m_viewOffset;
	}

private:
	ui::Canvas& m_canvas;
	ui::Rect m_rc;
	Vector2 m_viewOffset;
	float m_viewScale;
	float m_tail;

	ui::Point toPoint(const Vector2& p) const
	{
		return ui::Point(int32_t(p.x + 0.5f), int32_t(p.y + 0.5f));
	}

	Vector2 project(const Vector2& p) const
	{
		Vector2 pb = (p - m_viewOffset) / m_viewScale;
		pb = Vector2(m_rc.left, m_rc.top) + pb * Vector2(m_rc.getWidth(), m_rc.getHeight());
		return pb;
	}
};

Vector2 truncate(const Vector2& v)
{
	return Vector2(std::floor(v.x), std::floor(v.y));
}

bool compareSegmentsY(const Segment& ls, const Segment& rs)
{
	return ls.v[0].y < rs.v[0].y;
}

bool compareSegmentsX(const Segment& ls, const Segment& rs)
{
	if (ls.v[0].x < rs.v[0].x)
		return true;
	if (ls.v[0].x > rs.v[0].x)
		return false;

	if (ls.v[1].x < rs.v[1].x)
		return true;
	if (ls.v[1].x > rs.v[1].x)
		return false;

	return false;
}

void visualizeTriangulation(Painter& p, const AlignedVector< Segment >& segments, const ui::Point& mousePosition)
{
	std::set< float > pys;
	Segment s;
	Triangle t;

	Vector2 mp = p.clientToView(mousePosition);

	AlignedVector< Segment >m_segments;
	AlignedVector< Segment > m_slabs;

	// Create vertical segments.
	for (AlignedVector< Segment >::const_iterator i = segments.begin(); i != segments.end(); ++i)
	{
		if (!i->fillStyle0 && !i->fillStyle1)
			continue;

		if (!i->curve)
		{
			if (abs< float >(i->v[0].y - i->v[1].y) > 0.0f)
			{
				s.v[0] = i->v[0];
				s.v[1] = i->v[1];
				s.curve = false;
				s.fillStyle0 = i->fillStyle0;
				s.fillStyle1 = i->fillStyle1;
				m_segments.push_back(s);

				pys.insert(std::floor(s.v[0].y));
				pys.insert(std::floor(s.v[1].y));
			}
		}
		else
		{
			const Vector2& cp0 = i->v[0];
			const Vector2& cp1 = i->c;
			const Vector2& cp2 = i->v[1];

			Bezier2nd b(cp0, cp1, cp2);

			float Tlmmy = b.getLocalMinMaxY();
			if (Tlmmy > 0.0f && Tlmmy < 1.0f)
			{
				// A local min/max exist; need to split curve into two segments.
				Bezier2nd b0, b1;
				b.split(Tlmmy, b0, b1);

				b0.cp0 = truncate(b0.cp0);
				b0.cp1 = truncate(b0.cp1);
				b0.cp2 = truncate(b0.cp2);

				b1.cp0 = truncate(b1.cp0);
				b1.cp1 = truncate(b1.cp1);
				b1.cp2 = truncate(b1.cp2);

				if (abs< float >(b0.cp0.y - b0.cp2.y) > 0.0f)
				{
					s.v[0] = b0.cp0;
					s.v[1] = b0.cp2;
					s.c = b0.cp1;
					s.curve = true;
					s.fillStyle0 = i->fillStyle0;
					s.fillStyle1 = i->fillStyle1;
					m_segments.push_back(s);

					pys.insert(std::floor(s.v[0].y));
					pys.insert(std::floor(s.v[1].y));
				}

				if (abs< float >(b1.cp0.y - b1.cp2.y) > 0.0f)
				{
					s.v[0] = b1.cp0;
					s.v[1] = b1.cp2;
					s.c = b1.cp1;
					s.curve = true;
					s.fillStyle0 = i->fillStyle0;
					s.fillStyle1 = i->fillStyle1;
					m_segments.push_back(s);

					pys.insert(std::floor(s.v[0].y));
					pys.insert(std::floor(s.v[1].y));
				}
			}
			else
			{
				if (abs< float >(cp0.y - cp2.y) > 0.0f)
				{
					s.v[0] = cp0;
					s.v[1] = cp2;
					s.c = cp1;
					s.curve = true;
					s.fillStyle0 = i->fillStyle0;
					s.fillStyle1 = i->fillStyle1;
					m_segments.push_back(s);

					pys.insert(std::floor(s.v[0].y));
					pys.insert(std::floor(s.v[1].y));
				}
			}
		}
	}

	if (pys.empty())
		return;

	for (std::set< float >::iterator i = pys.begin(); i != pys.end(); ++i)
		p.drawHLine(*i, Color4ub(0, 0, 0, 20));

	// Ensure all segments are top to bottom.
	for (AlignedVector< Segment >::iterator i = m_segments.begin(); i != m_segments.end(); ++i)
	{
		if (i->v[0].y > i->v[1].y)
		{
			std::swap(i->v[0], i->v[1]);
			std::swap(i->fillStyle0, i->fillStyle1);
		}
	}

	// Sort segments to Y.
	std::sort(m_segments.begin(), m_segments.end(), compareSegmentsY);

	uint32_t tcount = 0;

	pys.erase(pys.begin());
	for (std::set< float >::iterator i = pys.begin(); i != pys.end(); ++i)
	{
		m_slabs.resize(0);

		for (AlignedVector< Segment >::iterator j = m_segments.begin(); j != m_segments.end(); )
		{
			// As segments are sorted we can safely abort if we find one that's completely below.
			if (j->v[0].y >= *i)
				break;

			if (!j->curve)
			{
				float d = j->v[1].y - j->v[0].y;
				if (d <= 0.0f)
				{
					++j;
					continue;
				}

				float t = (*i - j->v[0].y) / d;
				float x = j->v[0].x + (j->v[1].x - j->v[0].x) * t;

				s.v[0] = j->v[0];
				s.v[1] = Vector2(x, *i);
				s.curve = false;
				s.fillStyle0 = j->fillStyle0;
				s.fillStyle1 = j->fillStyle1;
				s.lineStyle = j->lineStyle;
				m_slabs.push_back(s);

				j->v[0] = s.v[1];
			}
			else
			{
				if (*i < j->v[1].y)
				{
					const Vector2& cp0 = j->v[0];
					const Vector2& cp1 = j->c;
					const Vector2& cp2 = j->v[1];

					Bezier2nd b(cp0, cp1, cp2);

					float t0, t1;
					b.intersectX(float(*i), t0, t1);

					T_ASSERT (t0 > -FUZZY_EPSILON && t0 < 1.0f + FUZZY_EPSILON);

					Bezier2nd b0, b1;
					b.split(t0, b0, b1);

					s.v[0] = b0.cp0;
					s.v[1] = b0.cp2;
					s.c = b0.cp1;
					s.curve = true;
					s.fillStyle0 = j->fillStyle0;
					s.fillStyle1 = j->fillStyle1;
					s.lineStyle = j->lineStyle;
					m_slabs.push_back(s);

					j->v[0] = b1.cp0;
					j->c = b1.cp1;
				}
				else
				{
					float d = j->v[1].y - j->v[0].y;
					if (d <= 0.0f)
					{
						++j;
						continue;
					}

					s.v[0] = j->v[0];
					s.v[1] = j->v[1];
					s.c = j->c;
					s.curve = true;
					s.fillStyle0 = j->fillStyle0;
					s.fillStyle1 = j->fillStyle1;
					s.lineStyle = j->lineStyle;
					m_slabs.push_back(s);

					j->v[0] = j->v[1];
				}
			}

			if (abs< float >(j->v[1].y - j->v[0].y) <= 0.0f)
				j = m_segments.erase(j);
			else
				j++;
		}

		if (m_slabs.empty())
			continue;

		std::sort(m_slabs.begin(), m_slabs.end(), compareSegmentsX);

		uint16_t fillStyle = 0;
		for (size_t i = 0; i < m_slabs.size() - 1; ++i)
		{
			const Segment& sl = m_slabs[i];
			const Segment& sr = m_slabs[i + 1];

			fillStyle = sl.fillStyle0;

			//if (i == 0)
			//	fillStyle = sl.fillStyle0 ? sl.fillStyle0 : sl.fillStyle1;
			//else
			//{
			//	if (sl.fillStyle0 == fillStyle)
			//		fillStyle = sl.fillStyle1;
			//	else
			//		fillStyle = sl.fillStyle0;
			//}

			if (mp.x >= sl.v[0].x && mp.x <= sr.v[0].x && mp.y >= sl.v[0].y && mp.y <= sl.v[1].y)
				log::info << fillStyle << Endl;

			if (fillStyle)
			{
				if (sl.v[0].x >= sr.v[0].x && sl.v[1].x >= sr.v[1].x)
					continue;

				float y0 = sl.v[0].y;
				float y1 = sl.v[1].y;
				
				if (y0 >= y1)
					continue;

				bool il = false, ir = false;

				if (sl.curve)
					p.drawCurve(Vector2(sl.v[0].x, y0), sl.c, Vector2(sl.v[1].x, y1), Color4ub(0, 0, 0, 255));
				else
					p.drawLine(Vector2(sl.v[0].x, y0), Vector2(sl.v[1].x, y1), Color4ub(0, 0, 0, 255));

				if (sr.curve)
					p.drawCurve(Vector2(sr.v[0].x, y0), sr.c, Vector2(sr.v[1].x, y1), Color4ub(0, 0, 0, 255));
				else
					p.drawLine(Vector2(sr.v[0].x, y0), Vector2(sr.v[1].x, y1), Color4ub(0, 0, 0, 255));



				if (sl.curve)
				{
					t.v[0] = Vector2(sl.v[0].x, y0);
					t.v[1] = sl.c;
					t.v[2] = Vector2(sl.v[1].x, y1);

					il = bool(Line2(t.v[0], t.v[2]).distance(sl.c) >= 0.0f);

					t.type = il ? TcOut : TcIn;
					t.fillStyle = fillStyle;
					p.drawTriangle(t);

					++tcount;
				}

				if (sr.curve)
				{
					t.v[0] = Vector2(sr.v[0].x, y0);
					t.v[1] = sr.c;
					t.v[2] = Vector2(sr.v[1].x, y1);

					ir = bool(Line2(t.v[0], t.v[2]).distance(sr.c) < 0.0f);

					t.type = ir ? TcOut : TcIn;
					t.fillStyle = fillStyle;
					p.drawTriangle(t);

					++tcount;
				}

				if (!il && !ir)
				{
					t.v[0] = Vector2(sl.v[0].x, y0);
					t.v[1] = Vector2(sr.v[0].x, y0);
					t.v[2] = Vector2(sl.v[1].x, y1);
					t.type = TcFill;
					t.fillStyle = fillStyle;
					p.drawTriangle(t);
					++tcount;

					t.v[0] = Vector2(sr.v[1].x, y1);
					t.v[1] = Vector2(sl.v[1].x, y1);
					t.v[2] = Vector2(sr.v[0].x, y0);
					t.type = TcFill;
					t.fillStyle = fillStyle;
					p.drawTriangle(t);
					++tcount;

				}
				else if (il && !ir)
				{
					t.v[0] = Vector2(sl.v[0].x, y0);
					t.v[1] = Vector2(sr.v[0].x, y0);
					t.v[2] = sl.c;
					t.type = TcFill;
					t.fillStyle = fillStyle;
					p.drawTriangle(t);
					++tcount;


					t.v[0] = Vector2(sr.v[0].x, y0);
					t.v[1] = Vector2(sr.v[1].x, y1);
					t.v[2] = sl.c;
					t.type = TcFill;
					t.fillStyle = fillStyle;
					p.drawTriangle(t);
					++tcount;


					t.v[0] = Vector2(sr.v[1].x, y1);
					t.v[1] = Vector2(sl.v[1].x, y1);
					t.v[2] = sl.c;
					t.type = TcFill;
					t.fillStyle = fillStyle;
					p.drawTriangle(t);
					++tcount;

				}
				else if (!il && ir)
				{
					t.v[0] = Vector2(sl.v[0].x, y0);
					t.v[1] = Vector2(sr.v[0].x, y0);
					t.v[2] = sr.c;
					t.type = TcFill;
					t.fillStyle = fillStyle;
					p.drawTriangle(t);
					++tcount;


					t.v[0] = Vector2(sl.v[1].x, y1);
					t.v[1] = Vector2(sl.v[0].x, y0);
					t.v[2] = sr.c;
					t.type = TcFill;
					t.fillStyle = fillStyle;
					p.drawTriangle(t);
					++tcount;


					t.v[0] = Vector2(sr.v[1].x, y1);
					t.v[1] = Vector2(sl.v[1].x, y1);
					t.v[2] = sr.c;
					t.type = TcFill;
					t.fillStyle = fillStyle;
					p.drawTriangle(t);
					++tcount;

				}
				else	// il && ir
				{
					t.v[0] = Vector2(sl.v[0].x, y0);
					t.v[1] = Vector2(sr.v[0].x, y0);
					t.v[2] = sl.c;
					t.type = TcFill;
					t.fillStyle = fillStyle;
					p.drawTriangle(t);
					++tcount;


					t.v[0] = Vector2(sr.v[0].x, y0);
					t.v[1] = sr.c;
					t.v[2] = sl.c;
					t.type = TcFill;
					t.fillStyle = fillStyle;
					p.drawTriangle(t);

					++tcount;


					t.v[0] = Vector2(sr.v[1].x, y1);
					t.v[1] = Vector2(sl.v[1].x, y1);
					t.v[2] = sr.c;
					t.type = TcFill;
					t.fillStyle = fillStyle;
					p.drawTriangle(t);
					++tcount;


					t.v[0] = Vector2(sl.v[1].x, y1);
					t.v[1] = sl.c;
					t.v[2] = sr.c;
					t.type = TcFill;
					t.fillStyle = fillStyle;
					p.drawTriangle(t);
					++tcount;

				}

			}
		}
	}

	log::info << tcount << L" triangles" << Endl;
}

void visualizeTriangulation(Painter& p, const Path& path, uint16_t fillStyle, const ui::Point& mousePosition)
{
	AlignedVector< Segment > segments;
	Segment s;

	const AlignedVector< Vector2 >& points = path.getPoints();
	const AlignedVector< SubPath >& subPaths = path.getSubPaths();

	//std::set< uint16_t > fillStyles;
	//for (uint32_t j = 0; j < subPaths.size(); ++j)
	//{
	//	const SubPath& sp = subPaths[j];
	//	if (sp.fillStyle0)
	//		fillStyles.insert(sp.fillStyle0);
	//	if (sp.fillStyle1)
	//		fillStyles.insert(sp.fillStyle1);
	//}

	//for (std::set< uint16_t >::const_iterator ii = fillStyles.begin(); ii != fillStyles.end(); ++ii)
	{
		for (uint32_t j = 0; j < subPaths.size(); ++j)
		{
			const SubPath& sp = subPaths[j];
			if (sp.fillStyle0 != /**ii*/fillStyle && sp.fillStyle1 != /**ii*/fillStyle)
				continue;

			for (AlignedVector< SubPathSegment >::const_iterator k = sp.segments.begin(); k != sp.segments.end(); ++k)
			{
				switch (k->type)
				{
				case SpgtLinear:
					{
						s.v[0] = points[k->pointsOffset];
						s.v[1] = points[k->pointsOffset + 1];
						s.curve = false;
						s.fillStyle0 = sp.fillStyle0;
						s.fillStyle1 = sp.fillStyle1;
						s.lineStyle = sp.lineStyle;
						segments.push_back(s);
					}
					break;

				case SpgtQuadratic:
					{
						s.v[0] = points[k->pointsOffset];
						s.v[1] = points[k->pointsOffset + 2];
						s.c = points[k->pointsOffset + 1];
						s.curve = true;
						s.fillStyle0 = sp.fillStyle0;
						s.fillStyle1 = sp.fillStyle1;
						s.lineStyle = sp.lineStyle;
						segments.push_back(s);
					}
					break;

				default:
					break;
				}
			}
		}

		if (!segments.empty())
		{
			visualizeTriangulation(p, segments, mousePosition);
			segments.resize(0);
		}
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashPathControl", FlashPathControl, ui::Widget)

bool FlashPathControl::create(ui::Widget* parent, int style)
{
	if (!ui::Widget::create(parent, style))
		return false;

	addEventHandler< ui::PaintEvent >(this, &FlashPathControl::eventPaint);
	addEventHandler< ui::MouseButtonDownEvent >(this, &FlashPathControl::eventMouseButtonDown);
	addEventHandler< ui::MouseButtonUpEvent >(this, &FlashPathControl::eventMouseButtonUp);
	addEventHandler< ui::MouseMoveEvent >(this, &FlashPathControl::eventMouseMove);
	addEventHandler< ui::MouseWheelEvent >(this, &FlashPathControl::eventMouseWheel);
	addEventHandler< ui::KeyDownEvent >(this, &FlashPathControl::eventKeyDown);

	m_viewOffset = Vector2::zero();
	m_viewScale = 1000.0f;

	return true;
}

void FlashPathControl::setPath(const Path& path)
{
	m_path = path;

	const AlignedVector< SubPath >& subPaths = path.getSubPaths();

	std::set< uint16_t > fillStyles;
	for (uint32_t j = 0; j < subPaths.size(); ++j)
	{
		const SubPath& sp = subPaths[j];
		if (sp.fillStyle0)
			fillStyles.insert(sp.fillStyle0);
		if (sp.fillStyle1)
			fillStyles.insert(sp.fillStyle1);
	}
	m_fillStyles = std::vector< uint16_t >(fillStyles.begin(), fillStyles.end());
	m_fillStyleIndex = 0;

	update();
}

void FlashPathControl::eventPaint(ui::PaintEvent* event)
{
	ui::Canvas& canvas = event->getCanvas();
	ui::Rect rc = getInnerRect();

	const ui::StyleSheet* ss = ui::Application::getInstance()->getStyleSheet();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rc);

	if (!m_fillStyles.empty())
	{
		Painter p(canvas, rc.inflate(-8, -8), m_viewOffset, m_viewScale);
		visualizeTriangulation(p, m_path, m_fillStyles[m_fillStyleIndex], m_lastMousePosition);
	}

	event->consume();
}

void FlashPathControl::eventMouseButtonDown(ui::MouseButtonDownEvent* event)
{
	m_lastMousePosition = event->getPosition();
	setCapture();
}

void FlashPathControl::eventMouseButtonUp(ui::MouseButtonUpEvent* event)
{
	releaseCapture();
}

void FlashPathControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	if (hasCapture())
	{
		ui::Size delta = event->getPosition() - m_lastMousePosition;
		m_lastMousePosition = event->getPosition();

		m_viewOffset -= Vector2(delta.cx, delta.cy);

		update();
	}
	else
	{
		m_lastMousePosition = event->getPosition();
		update();
	}
}

void FlashPathControl::eventMouseWheel(ui::MouseWheelEvent* event)
{
	m_viewScale += event->getRotation() * 100.0f;
	if (m_viewScale < 1.0f)
		m_viewScale = 1.0f;

	update();

	log::info << m_viewScale << Endl;
}

void FlashPathControl::eventKeyDown(ui::KeyDownEvent* event)
{
	m_fillStyleIndex = (m_fillStyleIndex + 1) % m_fillStyles.size();
	log::info << L"Showing fill style " << m_fillStyleIndex << Endl;

	update();
}

	}
}
