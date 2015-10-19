#include <algorithm>
#include <cfloat>
#include "Core/Log/Log.h"
#include "Core/Math/Bezier2nd.h"
#include "Core/Math/Const.h"
#include "Core/Math/Line2.h"
#include "Core/Math/MathUtils.h"
#include "Flash/Acc/Triangulator.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Triangulator", Triangulator, Object)

void Triangulator::triangulate(const AlignedVector< Segment >& segments, AlignedVector< Triangle >& outTriangles)
{
	std::set< float > pys;
	Segment s;
	Triangle t;

	m_segments.resize(0);

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

				pys.insert(s.v[0].y);
				pys.insert(s.v[1].y);
			}
		}
		else
		{
			const Vector2& cp0 = i->v[0];
			const Vector2& cp1 = i->c;
			const Vector2& cp2 = i->v[1];

			Bezier2nd b(cp0, cp1, cp2);

			float Tlmmy = b.getLocalMinMaxY();
			if (Tlmmy > FUZZY_EPSILON && Tlmmy < 1.0f - FUZZY_EPSILON)
			{
				// A local min/max exist; need to split curve into two segments.
				Bezier2nd b0, b1;
				b.split(Tlmmy, b0, b1);

				if (abs< float >(b0.cp0.y - b0.cp2.y) > 0.0f)
				{
					s.v[0] = b0.cp0;
					s.v[1] = b0.cp2;
					s.c = b0.cp1;
					s.curve = true;
					s.fillStyle0 = i->fillStyle0;
					s.fillStyle1 = i->fillStyle1;
					m_segments.push_back(s);

					pys.insert(s.v[0].y);
					pys.insert(s.v[1].y);
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

					pys.insert(s.v[0].y);
					pys.insert(s.v[1].y);
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

					pys.insert(s.v[0].y);
					pys.insert(s.v[1].y);
				}
			}
		}
	}

	if (pys.empty())
		return;

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

			if (i == 0)
				fillStyle = sl.fillStyle0;
			else
			{
				if (sl.fillStyle0 == fillStyle)
					fillStyle = sl.fillStyle1;
				else
					fillStyle = sl.fillStyle0;
			}

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
				{
					t.v[0] = Vector2(sl.v[0].x, y0);
					t.v[1] = sl.c;
					t.v[2] = Vector2(sl.v[1].x, y1);

					il = bool(Line2(t.v[0], t.v[2]).distance(sl.c) >= 0.0f);

					t.type = il ? TcOut : TcIn;
					t.fillStyle = fillStyle;
					outTriangles.push_back(t);
				}

				if (sr.curve)
				{
					t.v[0] = Vector2(sr.v[0].x, y0);
					t.v[1] = sr.c;
					t.v[2] = Vector2(sr.v[1].x, y1);

					ir = bool(Line2(t.v[0], t.v[2]).distance(sr.c) < 0.0f);

					t.type = ir ? TcOut : TcIn;
					t.fillStyle = fillStyle;
					outTriangles.push_back(t);
				}

				if (!il && !ir)
				{
					t.v[0] = Vector2(sl.v[0].x, y0);
					t.v[1] = Vector2(sr.v[0].x, y0);
					t.v[2] = Vector2(sl.v[1].x, y1);
					t.type = TcFill;
					t.fillStyle = fillStyle;
					outTriangles.push_back(t);

					t.v[0] = Vector2(sr.v[1].x, y1);
					t.v[1] = Vector2(sl.v[1].x, y1);
					t.v[2] = Vector2(sr.v[0].x, y0);
					t.type = TcFill;
					t.fillStyle = fillStyle;
					outTriangles.push_back(t);
				}
				else if (il && !ir)
				{
					t.v[0] = Vector2(sl.v[0].x, y0);
					t.v[1] = Vector2(sr.v[0].x, y0);
					t.v[2] = sl.c;
					t.type = TcFill;
					t.fillStyle = fillStyle;
					outTriangles.push_back(t);

					t.v[0] = Vector2(sr.v[0].x, y0);
					t.v[1] = Vector2(sr.v[1].x, y1);
					t.v[2] = sl.c;
					t.type = TcFill;
					t.fillStyle = fillStyle;
					outTriangles.push_back(t);

					t.v[0] = Vector2(sr.v[1].x, y1);
					t.v[1] = Vector2(sl.v[1].x, y1);
					t.v[2] = sl.c;
					t.type = TcFill;
					t.fillStyle = fillStyle;
					outTriangles.push_back(t);
				}
				else if (!il && ir)
				{
					t.v[0] = Vector2(sl.v[0].x, y0);
					t.v[1] = Vector2(sr.v[0].x, y0);
					t.v[2] = sr.c;
					t.type = TcFill;
					t.fillStyle = fillStyle;
					outTriangles.push_back(t);

					t.v[0] = Vector2(sl.v[1].x, y1);
					t.v[1] = Vector2(sl.v[0].x, y0);
					t.v[2] = sr.c;
					t.type = TcFill;
					t.fillStyle = fillStyle;
					outTriangles.push_back(t);

					t.v[0] = Vector2(sr.v[1].x, y1);
					t.v[1] = Vector2(sl.v[1].x, y1);
					t.v[2] = sr.c;
					t.type = TcFill;
					t.fillStyle = fillStyle;
					outTriangles.push_back(t);
				}
				else	// il && ir
				{
					t.v[0] = Vector2(sl.v[0].x, y0);
					t.v[1] = Vector2(sr.v[0].x, y0);
					t.v[2] = sl.c;
					t.type = TcFill;
					t.fillStyle = fillStyle;
					outTriangles.push_back(t);

					t.v[0] = Vector2(sr.v[0].x, y0);
					t.v[1] = sr.c;
					t.v[2] = sl.c;
					t.type = TcFill;
					t.fillStyle = fillStyle;
					outTriangles.push_back(t);

					t.v[0] = Vector2(sr.v[1].x, y1);
					t.v[1] = Vector2(sl.v[1].x, y1);
					t.v[2] = sr.c;
					t.type = TcFill;
					t.fillStyle = fillStyle;
					outTriangles.push_back(t);

					t.v[0] = Vector2(sl.v[1].x, y1);
					t.v[1] = sl.c;
					t.v[2] = sr.c;
					t.type = TcFill;
					t.fillStyle = fillStyle;
					outTriangles.push_back(t);
				}
			}
		}
	}
}

	}
}
