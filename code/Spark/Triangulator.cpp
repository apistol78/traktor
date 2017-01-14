#include <algorithm>
#include <cfloat>
#include "Core/Log/Log.h"
#include "Core/Math/Bezier2nd.h"
#include "Core/Math/Bezier3rd.h"
#include "Core/Math/Const.h"
#include "Core/Math/Line2.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Ray2.h"
#include "Spark/Path.h"
#include "Spark/Triangulator.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

const float c_approximateTolerance = 10.0f;

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

void Triangulator::fill(const Path* path, AlignedVector< Triangle >& outTriangles)
{
	AlignedVector< Segment > segments;
	Segment s;

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
					s.curve = false;
					s.v[0] = i->points[j];
					s.v[1] = i->points[j + 1];
					segments.push_back(s);
				}
			}
			break;

		case SptQuadric:
			{
				for (uint32_t j = 0; j < i->points.size() - 1; j += 2)
				{
					s.curve = true;
					s.v[0] = i->points[j];
					s.v[1] = i->points[j + 2];
					s.c = i->points[j + 1];
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
					b.approximate(c_approximateTolerance, 4, a);

					for (AlignedVector< Bezier2nd >::const_iterator k = a.begin(); k != a.end(); ++k)
					{
						s.curve = true;
						s.v[0] = k->cp0;
						s.v[1] = k->cp2;
						s.c = k->cp1;
						segments.push_back(s);
					}
				}
			}
			break;

		default:
			break;
		}

		if (lastSubPath)
		{
			s.curve = false;
			s.v[0] = i->points.back();
			s.v[1] = i->origin;
			segments.push_back(s);
		}
	}

	// Create triangles from segments.
	triangulate(segments, outTriangles);
}

void Triangulator::stroke(const Path* path, float width, AlignedVector< Triangle >& outTriangles)
{
	AlignedVector< Segment > segments;
	Segment s;
	float r, k;

	float halfWidth = width / 2.0f;

	// Create segments from path.
	const AlignedVector< SubPath >& subPaths = path->getSubPaths();
	for (size_t i = 0; i < subPaths.size(); ++i)
	{
		const SubPath& sp = subPaths[i];

		bool first = bool(i == 0);
		bool last = bool(i == subPaths.size() - 1);

		if (first)
		{
			Vector2 d = (sp.points[1] - sp.points[0]).normalized();
			Vector2 p = d.perpendicular();
			
			s.curve = false;
			s.v[0] = sp.points[0] - p * halfWidth;
			s.v[1] = sp.points[0] + p * halfWidth;
			segments.push_back(s);
		}

		switch (sp.type)
		{
		case SptLinear:
			{
				Vector2 d_01 = (sp.points[1] - sp.points[0]).normalized();
				Vector2 p_01_l = sp.points[0] - d_01.perpendicular() * halfWidth;
				Vector2 p_01_r = sp.points[0] + d_01.perpendicular() * halfWidth;

				for (uint32_t j = 1; j < sp.points.size() - 1; ++j)
				{
					Vector2 d_12 = (sp.points[j + 1] - sp.points[j]).normalized();
					Vector2 p_12_l = sp.points[j] - d_12.perpendicular() * halfWidth;
					Vector2 p_12_r = sp.points[j] + d_12.perpendicular() * halfWidth;

					Vector2 p_l;
					if (Ray2(p_01_l, d_01).intersect(Ray2(p_12_l, d_12), r, k))
						p_l = p_01_l + r * d_01;
					else
						p_l = p_12_l;

					Vector2 p_r;
					if (Ray2(p_01_r, d_01).intersect(Ray2(p_12_r, d_12), r, k))
						p_r = p_01_r + r * d_01;
					else
						p_r = p_12_r;

					s.curve = false;
					s.v[0] = p_01_l;
					s.v[1] = p_l;
					segments.push_back(s);

					s.curve = false;
					s.v[0] = p_01_r;
					s.v[1] = p_r;
					segments.push_back(s);

					d_01 = d_12;
					p_01_l = p_l;
					p_01_r = p_r;
				}
			}
			break;

		default:
			break;
		}

		if (last)
		{
			size_t np = sp.points.size() - 1;

			Vector2 d = (sp.points[np] - sp.points[np - 1]).normalized();
			Vector2 p = d.perpendicular();
			
			s.curve = false;
			s.v[0] = sp.points[np] - p * halfWidth;
			s.v[1] = sp.points[np] + p * halfWidth;
			segments.push_back(s);
		}
	}

	// Create triangles from segments.
	triangulate(segments, outTriangles);
}

void Triangulator::triangulate(const AlignedVector< Segment >& segments, AlignedVector< Triangle >& outTriangles)
{
	std::set< float > pys;
	Segment s;
	Triangle t;

	m_segments.resize(0);

	// Create vertical segments.
	for (AlignedVector< Segment >::const_iterator i = segments.begin(); i != segments.end(); ++i)
	{
		if (!i->curve)
		{
			if (abs< float >(i->v[0].y - i->v[1].y) > 0.0f)
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
					s.curve = true;
					s.v[0] = b0.cp0;
					s.v[1] = b0.cp2;
					s.c = b0.cp1;
					m_segments.push_back(s);

					pys.insert(s.v[0].y);
					pys.insert(s.v[1].y);
				}

				if (abs< float >(b1.cp0.y - b1.cp2.y) > 0.0f)
				{
					s.curve = true;
					s.v[0] = b1.cp0;
					s.v[1] = b1.cp2;
					s.c = b1.cp1;
					m_segments.push_back(s);

					pys.insert(s.v[0].y);
					pys.insert(s.v[1].y);
				}
			}
			else
			{
				if (abs< float >(cp0.y - cp2.y) > 0.0f)
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

				s.curve = false;
				s.v[0] = j->v[0];
				s.v[1] = Vector2(int32_t(x), *i);
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
					b.intersectX(*i, t0, t1);

					T_ASSERT (t0 > -FUZZY_EPSILON && t0 < 1.0f + FUZZY_EPSILON);

					Bezier2nd b0, b1;
					b.split(t0, b0, b1);

					s.curve = true;
					s.v[0] = b0.cp0;
					s.v[1] = b0.cp2;
					s.c = b0.cp1;
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

					s.curve = true;
					s.v[0] = j->v[0];
					s.v[1] = j->v[1];
					s.c = j->c;
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

		for (size_t i = 0; i < m_slabs.size() - 1; ++i)
		{
			const Segment& sl = m_slabs[i];
			const Segment& sr = m_slabs[i + 1];

			if ((i & 1) == 0)
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

					outTriangles.push_back(t);
				}

				if (sr.curve)
				{
					t.v[0] = Vector2(sr.v[0].x, y0);
					t.v[1] = sr.c;
					t.v[2] = Vector2(sr.v[1].x, y1);

					ir = bool(Line2(t.v[0], t.v[2]).distance(sr.c) < 0.0f);
					t.type = ir ? TcOut : TcIn;

					outTriangles.push_back(t);
				}

				if (!il && !ir)
				{
					t.v[0] = Vector2(sl.v[0].x, y0);
					t.v[1] = Vector2(sr.v[0].x, y0);
					t.v[2] = Vector2(sl.v[1].x, y1);
					t.type = TcFill;
					outTriangles.push_back(t);

					t.v[0] = Vector2(sr.v[1].x, y1);
					t.v[1] = Vector2(sl.v[1].x, y1);
					t.v[2] = Vector2(sr.v[0].x, y0);
					t.type = TcFill;
					outTriangles.push_back(t);
				}
				else if (il && !ir)
				{
					t.v[0] = Vector2(sl.v[0].x, y0);
					t.v[1] = Vector2(sr.v[0].x, y0);
					t.v[2] = sl.c;
					t.type = TcFill;
					outTriangles.push_back(t);

					t.v[0] = Vector2(sr.v[0].x, y0);
					t.v[1] = Vector2(sr.v[1].x, y1);
					t.v[2] = sl.c;
					t.type = TcFill;
					outTriangles.push_back(t);

					t.v[0] = Vector2(sr.v[1].x, y1);
					t.v[1] = Vector2(sl.v[1].x, y1);
					t.v[2] = sl.c;
					t.type = TcFill;
					outTriangles.push_back(t);
				}
				else if (!il && ir)
				{
					t.v[0] = Vector2(sl.v[0].x, y0);
					t.v[1] = Vector2(sr.v[0].x, y0);
					t.v[2] = sr.c;
					t.type = TcFill;
					outTriangles.push_back(t);

					t.v[0] = Vector2(sl.v[1].x, y1);
					t.v[1] = Vector2(sl.v[0].x, y0);
					t.v[2] = sr.c;
					t.type = TcFill;
					outTriangles.push_back(t);

					t.v[0] = Vector2(sr.v[1].x, y1);
					t.v[1] = Vector2(sl.v[1].x, y1);
					t.v[2] = sr.c;
					t.type = TcFill;
					outTriangles.push_back(t);
				}
				else	// il && ir
				{
					t.v[0] = Vector2(sl.v[0].x, y0);
					t.v[1] = Vector2(sr.v[0].x, y0);
					t.v[2] = sl.c;
					t.type = TcFill;
					outTriangles.push_back(t);

					t.v[0] = Vector2(sr.v[0].x, y0);
					t.v[1] = sr.c;
					t.v[2] = sl.c;
					t.type = TcFill;
					outTriangles.push_back(t);

					t.v[0] = Vector2(sr.v[1].x, y1);
					t.v[1] = Vector2(sl.v[1].x, y1);
					t.v[2] = sr.c;
					t.type = TcFill;
					outTriangles.push_back(t);

					t.v[0] = Vector2(sl.v[1].x, y1);
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
