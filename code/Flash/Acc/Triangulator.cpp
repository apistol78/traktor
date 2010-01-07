#include <algorithm>
#include <cfloat>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Triangulator.h"
#include "Flash/Acc/Triangulator.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

struct Trapezoid
{
	float y[2];
	float lx[2];
	float rx[2];
	uint16_t fillStyle;
};

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

	m_segments.resize(0);
	for (AlignedVector< Segment >::const_iterator i = segments.begin(); i != segments.end(); ++i)
	{
		if (!i->fillStyle0 && !i->fillStyle1)
			continue;
		if (std::abs(i->v[1].y - i->v[0].y) <= FUZZY_EPSILON)
			continue;

		Segment segment;

		if (i->v[0].y < i->v[1].y)
		{
			segment.v[0] = i->v[0];
			segment.v[1] = i->v[1];
			segment.fillStyle0 = i->fillStyle0;
			segment.fillStyle1 = i->fillStyle1;
		}
		else if (i->v[0].y > i->v[1].y)
		{
			segment.v[0] = i->v[1];
			segment.v[1] = i->v[0];
			segment.fillStyle0 = i->fillStyle1;
			segment.fillStyle1 = i->fillStyle0;
		}

		m_segments.push_back(segment);
		pys.insert(segment.v[0].y);
		pys.insert(segment.v[1].y);
	}

	if (pys.empty())
		return;

	// Sort segments to Y.
	std::sort(m_segments.begin(), m_segments.end(), compareSegmentsY);

	// Create trapezoids.
	static AlignedVector< Trapezoid > trapezoids;
	trapezoids.resize(0);

	pys.erase(pys.begin());
	for (std::set< float >::iterator i = pys.begin(); i != pys.end(); ++i)
	{
		static AlignedVector< Segment > slabs;
		slabs.resize(0);

		for (AlignedVector< Segment >::iterator j = m_segments.begin(); j != m_segments.end(); )
		{
			// As segments are sorted we can safely abort if we find one that's completely below.
			if (j->v[0].y >= *i - FUZZY_EPSILON)
				break;

			float d = (j->v[1].y - j->v[0].y);
			if (d <= 0.0f)
				d = 1.0f;

			float t = (*i - j->v[0].y) / d;
			if (t > 1.0f)
				t = 1.0f;

			float x = j->v[0].x + (j->v[1].x - j->v[0].x) * t;

			Segment slab;
			slab.v[0] = j->v[0];
			slab.v[1] = Vector2(x, *i);
			slab.fillStyle0 = j->fillStyle0;
			slab.fillStyle1 = j->fillStyle1;
			slab.lineStyle = j->lineStyle;
			slabs.push_back(slab);

			j->v[0] = slab.v[1];

			if (std::abs(j->v[1].y - j->v[0].y) <= FUZZY_EPSILON)
				j = m_segments.erase(j);
			else
				j++;
		}
		if (slabs.empty())
			continue;

		std::sort(slabs.begin(), slabs.end(), compareSegmentsX);

		uint16_t fillStyle = 0;
		for (size_t i = 0; i < slabs.size() - 1; ++i)
		{
			const Segment& sl = slabs[i];
			const Segment& sr = slabs[i + 1];

			if (i == 0)
				fillStyle = sl.fillStyle0 ? sl.fillStyle0 : sl.fillStyle1;
			else
			{
				if (sl.fillStyle0 == fillStyle)
					fillStyle = sl.fillStyle1;
				else if (sl.fillStyle1 == fillStyle)
					fillStyle = sl.fillStyle0;
				else
					fillStyle = sl.fillStyle0 ? sl.fillStyle0 : sl.fillStyle1;
			}

			if (fillStyle)
			{
				if (sl.v[0].x >= sr.v[0].x && sl.v[1].x >= sr.v[1].x)
					continue;

				Trapezoid tr;
				
				tr.y[0] = sl.v[0].y;
				tr.y[1] = sl.v[1].y;
				tr.lx[0] = sl.v[0].x;
				tr.lx[1] = sl.v[1].x;
				tr.rx[0] = sr.v[0].x;
				tr.rx[1] = sr.v[1].x;
				tr.fillStyle = fillStyle;

				trapezoids.push_back(tr);
			}
		}
	}

	// Create triangles from trapezoids.
	outTriangles.resize(trapezoids.size() * 2);
	for (uint32_t i = 0; i < uint32_t(trapezoids.size()); ++i)
	{
		const Trapezoid& tz = trapezoids[i];

		outTriangles[i * 2 + 0].fillStyle = tz.fillStyle;
		outTriangles[i * 2 + 0].v[0] = Vector2(tz.lx[0], tz.y[0]);
		outTriangles[i * 2 + 0].v[1] = Vector2(tz.rx[0], tz.y[0]);
		outTriangles[i * 2 + 0].v[2] = Vector2(tz.rx[1], tz.y[1]);

		outTriangles[i * 2 + 1].fillStyle = tz.fillStyle;
		outTriangles[i * 2 + 1].v[0] = Vector2(tz.lx[0], tz.y[0]);
		outTriangles[i * 2 + 1].v[1] = Vector2(tz.rx[1], tz.y[1]);
		outTriangles[i * 2 + 1].v[2] = Vector2(tz.lx[1], tz.y[1]);
	}
}

	}
}
