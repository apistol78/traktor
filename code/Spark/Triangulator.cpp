#include <algorithm>
#include <cfloat>
#include "Core/Log/Log.h"
#include "Core/Math/Bezier2nd.h"
#include "Core/Math/Bezier3rd.h"
#include "Core/Math/Const.h"
#include "Core/Math/Line2.h"
#include "Core/Math/MathUtils.h"
#include "Spark/Path.h"
#include "Spark/Triangulator.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

bool compareSegmentsY(const Triangulator::Segment& ls, const Triangulator::Segment& rs)
{
	return ls.v[0].y < rs.v[0].y;
}

bool compareSegmentsX(const Triangulator::Segment& ls, const Triangulator::Segment& rs)
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Triangulator", Triangulator, Object)

void Triangulator::triangulate(const Path* path, AlignedVector< Triangle >& outTriangles)
{
	const float c_pointScale = 1.0f;
	const float m_cubicApproximationError = 1.0f;

	AlignedVector< Segment > segments;

	// Create segments from path.
	const AlignedVector< SubPath >& subPaths = path->getSubPaths();
	for (AlignedVector< SubPath >::const_iterator i = subPaths.begin(); i != subPaths.end(); ++i)
	{
		bool lastSubPath = bool(i == subPaths.end() - 1);
		switch (i->type)
		{
		case SptLinear:
			{
				for (uint32_t j = 0; j < i->points.size() - 1; ++j)
				{
					Triangulator::Segment s;
					s.curve = false;
					s.v[0] = Vector2i::fromVector2(i->points[j] * c_pointScale);
					s.v[1] = Vector2i::fromVector2(i->points[j + 1] * c_pointScale);
					segments.push_back(s);
				}
			}
			break;

		case SptQuadric:
			{
				for (uint32_t j = 0; j < i->points.size() - 1; j += 2)
				{
					Triangulator::Segment s;
					s.curve = true;
					s.v[0] = Vector2i::fromVector2(i->points[j] * c_pointScale);
					s.v[1] = Vector2i::fromVector2(i->points[j + 2] * c_pointScale);
					s.c = Vector2i::fromVector2(i->points[j + 1] * c_pointScale);
					segments.push_back(s);
				}
			}
			break;

		case SptCubic:
			{
				for (uint32_t j = 0; j < i->points.size() - 1; j += 3)
				{
					Bezier3rd b(
						i->points[j],
						i->points[j + 1],
						i->points[j + 2],
						i->points[j + 3]
					);

					AlignedVector< Bezier2nd > a;
					b.approximate(m_cubicApproximationError, 4, a);

					for (AlignedVector< Bezier2nd >::const_iterator k = a.begin(); k != a.end(); ++k)
					{
						Triangulator::Segment s;
						s.curve = true;
						s.v[0] = Vector2i::fromVector2(k->cp0 * c_pointScale);
						s.v[1] = Vector2i::fromVector2(k->cp2 * c_pointScale);
						s.c = Vector2i::fromVector2(k->cp1 * c_pointScale);
						segments.push_back(s);
					}
				}
			}
			break;
		}

		if (lastSubPath)
		{
			Triangulator::Segment s;
			s.curve = false;
			s.v[0] = Vector2i::fromVector2(i->points.back() * c_pointScale);
			s.v[1] = Vector2i::fromVector2(i->origin * c_pointScale);
			segments.push_back(s);
		}
	}

	// Create triangles from segments.
	triangulate(segments, outTriangles);
}

void Triangulator::triangulate(const AlignedVector< Segment >& segments, AlignedVector< Triangle >& outTriangles)
{
	std::set< int32_t > pys;
	Segment s;
	Triangle t;

	m_segments.resize(0);

	// Create vertical segments.
	for (AlignedVector< Segment >::const_iterator i = segments.begin(); i != segments.end(); ++i)
	{
		if (!i->curve)
		{
			if (abs< int32_t >(i->v[0].y - i->v[1].y) > 0)
			{
				s.curve = false;
				s.v[0] = i->v[0];
				s.v[1] = i->v[1];
				m_segments.push_back(s);

				pys.insert(s.v[0].y);
				pys.insert(s.v[1].y);
			}
		}
		else
		{
			const Vector2i& cp0 = i->v[0];
			const Vector2i& cp1 = i->c;
			const Vector2i& cp2 = i->v[1];

			Bezier2nd b(
				cp0.toVector2(),
				cp1.toVector2(),
				cp2.toVector2()
			);

			float Tlmmy = b.getLocalMinMaxY();
			if (Tlmmy > FUZZY_EPSILON && Tlmmy < 1.0f - FUZZY_EPSILON)
			{
				// A local min/max exist; need to split curve into two segments.
				Bezier2nd b0, b1;
				b.split(Tlmmy, b0, b1);

				Vector2i b0_cp0 = Vector2i::fromVector2(b0.cp0);
				Vector2i b0_cp1 = Vector2i::fromVector2(b0.cp1);
				Vector2i b0_cp2 = Vector2i::fromVector2(b0.cp2);

				if (abs< int32_t >(b0_cp0.y - b0_cp2.y) > 0)
				{
					s.curve = true;
					s.v[0] = b0_cp0;
					s.v[1] = b0_cp2;
					s.c = b0_cp1;
					m_segments.push_back(s);

					pys.insert(s.v[0].y);
					pys.insert(s.v[1].y);
				}

				Vector2i b1_cp0 = Vector2i::fromVector2(b1.cp0);
				Vector2i b1_cp1 = Vector2i::fromVector2(b1.cp1);
				Vector2i b1_cp2 = Vector2i::fromVector2(b1.cp2);

				if (abs< int32_t >(b1_cp0.y - b1_cp2.y) > 0)
				{
					s.curve = true;
					s.v[0] = b1_cp0;
					s.v[1] = b1_cp2;
					s.c = b1_cp1;
					m_segments.push_back(s);

					pys.insert(s.v[0].y);
					pys.insert(s.v[1].y);
				}
			}
			else
			{
				if (abs< int32_t >(cp0.y - cp2.y) > 0)
				{
					s.curve = true;
					s.v[0] = cp0;
					s.v[1] = cp2;
					s.c = cp1;
					m_segments.push_back(s);

					pys.insert(s.v[0].y);
					pys.insert(s.v[1].y);
				}
			}
		}
	}

	if (pys.empty())
		return;

	// Ensure all segments are locally top to bottom.
	for (AlignedVector< Segment >::iterator i = m_segments.begin(); i != m_segments.end(); ++i)
	{
		if (i->v[0].y > i->v[1].y)
			std::swap(i->v[0], i->v[1]);
	}

	// Sort segments to Y.
	std::sort(m_segments.begin(), m_segments.end(), compareSegmentsY);

	pys.erase(pys.begin());
	for (std::set< int32_t >::iterator i = pys.begin(); i != pys.end(); ++i)
	{
		m_slabs.resize(0);

		for (AlignedVector< Segment >::iterator j = m_segments.begin(); j != m_segments.end(); )
		{
			// As segments are sorted we can safely abort if we find one that's completely below.
			if (j->v[0].y >= *i)
				break;

			if (!j->curve)
			{
				int32_t d = j->v[1].y - j->v[0].y;
				if (d <= 0)
				{
					++j;
					continue;
				}

				float t = float(*i - j->v[0].y) / d;
				float x = j->v[0].x + (j->v[1].x - j->v[0].x) * t;

				s.curve = false;
				s.v[0] = j->v[0];
				s.v[1] = Vector2i(int32_t(x), *i);
				m_slabs.push_back(s);

				j->v[0] = s.v[1];
			}
			else
			{
				if (*i < j->v[1].y)
				{
					const Vector2i& cp0 = j->v[0];
					const Vector2i& cp1 = j->c;
					const Vector2i& cp2 = j->v[1];

					Bezier2nd b(
						cp0.toVector2(),
						cp1.toVector2(),
						cp2.toVector2()
					);

					float t0, t1;
					b.intersectX(*i, t0, t1);

					T_ASSERT (t0 > -FUZZY_EPSILON && t0 < 1.0f + FUZZY_EPSILON);

					Bezier2nd b0, b1;
					b.split(t0, b0, b1);

					Vector2i b0_cp0 = Vector2i::fromVector2(b0.cp0);
					Vector2i b0_cp1 = Vector2i::fromVector2(b0.cp1);
					Vector2i b0_cp2 = Vector2i::fromVector2(b0.cp2);

					Vector2i b1_cp0 = Vector2i::fromVector2(b1.cp0);
					Vector2i b1_cp1 = Vector2i::fromVector2(b1.cp1);
					Vector2i b1_cp2 = Vector2i::fromVector2(b1.cp2);

					s.curve = true;
					s.v[0] = b0_cp0;
					s.v[1] = b0_cp2;
					s.c = b0_cp1;
					m_slabs.push_back(s);

					j->v[0] = b1_cp0;
					j->c = b1_cp1;
				}
				else
				{
					int32_t d = j->v[1].y - j->v[0].y;
					if (d <= 0)
					{
						++j;
						continue;
					}

					s.curve = true;
					s.v[0] = j->v[0];
					s.v[1] = j->v[1];
					s.c = j->c;
					m_slabs.push_back(s);

					j->v[0] = j->v[1];
				}

			}

			if (abs< int32_t >(j->v[1].y - j->v[0].y) <= 0)
				j = m_segments.erase(j);
			else
				j++;
		}

		if (m_slabs.empty())
			continue;

		std::sort(m_slabs.begin(), m_slabs.end(), compareSegmentsX);

		for (size_t i = 0; i < m_slabs.size() - 1; ++i)
		{
			const Segment& sl = m_slabs[i];
			const Segment& sr = m_slabs[i + 1];

			if ((i & 1) == 0)
			{
				if (sl.v[0].x >= sr.v[0].x && sl.v[1].x >= sr.v[1].x)
					continue;

				int32_t y0 = sl.v[0].y;
				int32_t y1 = sl.v[1].y;
				
				if (y0 >= y1)
					continue;

				bool il = false, ir = false;

				if (sl.curve)
				{
					t.v[0] = Vector2i(sl.v[0].x, y0);
					t.v[1] = sl.c;
					t.v[2] = Vector2i(sl.v[1].x, y1);

					il = bool(Line2(t.v[0].toVector2(), t.v[2].toVector2()).distance(sl.c.toVector2()) >= 0.0f);
					t.type = il ? TcOut : TcIn;

					outTriangles.push_back(t);
				}

				if (sr.curve)
				{
					t.v[0] = Vector2i(sr.v[0].x, y0);
					t.v[1] = sr.c;
					t.v[2] = Vector2i(sr.v[1].x, y1);

					ir = bool(Line2(t.v[0].toVector2(), t.v[2].toVector2()).distance(sr.c.toVector2()) < 0.0f);
					t.type = ir ? TcOut : TcIn;

					outTriangles.push_back(t);
				}

				if (!il && !ir)
				{
					t.v[0] = Vector2i(sl.v[0].x, y0);
					t.v[1] = Vector2i(sr.v[0].x, y0);
					t.v[2] = Vector2i(sl.v[1].x, y1);
					t.type = TcFill;
					outTriangles.push_back(t);

					t.v[0] = Vector2i(sr.v[1].x, y1);
					t.v[1] = Vector2i(sl.v[1].x, y1);
					t.v[2] = Vector2i(sr.v[0].x, y0);
					t.type = TcFill;
					outTriangles.push_back(t);
				}
				else if (il && !ir)
				{
					t.v[0] = Vector2i(sl.v[0].x, y0);
					t.v[1] = Vector2i(sr.v[0].x, y0);
					t.v[2] = sl.c;
					t.type = TcFill;
					outTriangles.push_back(t);

					t.v[0] = Vector2i(sr.v[0].x, y0);
					t.v[1] = Vector2i(sr.v[1].x, y1);
					t.v[2] = sl.c;
					t.type = TcFill;
					outTriangles.push_back(t);

					t.v[0] = Vector2i(sr.v[1].x, y1);
					t.v[1] = Vector2i(sl.v[1].x, y1);
					t.v[2] = sl.c;
					t.type = TcFill;
					outTriangles.push_back(t);
				}
				else if (!il && ir)
				{
					t.v[0] = Vector2i(sl.v[0].x, y0);
					t.v[1] = Vector2i(sr.v[0].x, y0);
					t.v[2] = sr.c;
					t.type = TcFill;
					outTriangles.push_back(t);

					t.v[0] = Vector2i(sl.v[1].x, y1);
					t.v[1] = Vector2i(sl.v[0].x, y0);
					t.v[2] = sr.c;
					t.type = TcFill;
					outTriangles.push_back(t);

					t.v[0] = Vector2i(sr.v[1].x, y1);
					t.v[1] = Vector2i(sl.v[1].x, y1);
					t.v[2] = sr.c;
					t.type = TcFill;
					outTriangles.push_back(t);
				}
				else	// il && ir
				{
					t.v[0] = Vector2i(sl.v[0].x, y0);
					t.v[1] = Vector2i(sr.v[0].x, y0);
					t.v[2] = sl.c;
					t.type = TcFill;
					outTriangles.push_back(t);

					t.v[0] = Vector2i(sr.v[0].x, y0);
					t.v[1] = sr.c;
					t.v[2] = sl.c;
					t.type = TcFill;
					outTriangles.push_back(t);

					t.v[0] = Vector2i(sr.v[1].x, y1);
					t.v[1] = Vector2i(sl.v[1].x, y1);
					t.v[2] = sr.c;
					t.type = TcFill;
					outTriangles.push_back(t);

					t.v[0] = Vector2i(sl.v[1].x, y1);
					t.v[1] = sl.c;
					t.v[2] = sr.c;
					t.type = TcFill;
					outTriangles.push_back(t);
				}
			}
		}
	}
}

	}
}
