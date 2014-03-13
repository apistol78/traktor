#include <algorithm>
#include "Core/Math/Winding2.h"

namespace traktor
{
	namespace
	{

struct ChainSortPred
{
	T_MATH_INLINE bool operator () (const Vector2& lh, const Vector2& rh) const
	{
		return lh.x < rh.x || (lh.x == rh.x && lh.y < rh.y);
	}
};	

T_MATH_INLINE float isLeft(const Vector2& P0, const Vector2& P1, const Vector2& P2)
{
	return (P1.x - P0.x) * (P2.y - P0.y) - (P2.x - P0.x) * (P1.y - P0.y);
}

	}

Winding2 Winding2::convexHull(const Vector2* pnts, int npnts)
{
	Winding2 hull;
	
	// Sort points in increasing x- and y-coordinates.
	AlignedVector< Vector2 > P(npnts);
	for (int i = 0; i < npnts; ++i)
		P[i] = pnts[i];
	std::sort(P.begin(), P.end(), ChainSortPred());
	
	hull.points.resize(2 * npnts);

	int k = 0;
	for (int i = 0; i < npnts; ++i)
	{
		while (k >= 2 && isLeft(hull.points[k - 2], hull.points[k - 1], P[i]) <= 0.0f)
			--k;
		hull.points[k++] = P[i];
	}
	for (int i = npnts - 2, t = k + 1; i >= 0; --i)
	{
		while (k >= t && isLeft(hull.points[k - 2], hull.points[k - 1], P[i]) <= 0.0f)
			--k;
		hull.points[k++] = P[i];
	}
	
	if (k > 0)
		--k;

	hull.points.resize(k);
	return hull;
}

Winding2 Winding2::convexHull(const AlignedVector< Vector2 >& pnts)
{
	return convexHull(&pnts[0], int(pnts.size()));
}

bool Winding2::inside(const Vector2& pnt) const
{
	bool c = false;
	for (int32_t i = 0, j = int32_t(points.size()) - 1; i < int32_t(points.size()); j = i++)
	{
		if ((((points[i].y <= pnt.y) && (pnt.y < points[j].y)) || ((points[j].y <= pnt.y) && (pnt.y < points[i].y))) && (pnt.x < (points[j].x - points[i].x) * (pnt.y - points[i].y) / (points[j].y - points[i].y) + points[i].x))
			c = !c;
	}
	return c;
}

}
