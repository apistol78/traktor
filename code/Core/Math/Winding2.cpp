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
		if (lh.x < rh.x)
			return true;
		else if (lh.x > rh.x)
			return false;
		else if (lh.y < rh.y)
			return true;
		else if (lh.y > rh.y)
			return false;
		else
			return false;
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
	
	// Allocate hull for maximum number of points.
	hull.p.resize(npnts);
	
	// Sort points in increasing x- and y-coordinates.
	AlignedVector< Vector2 > P(npnts);
	for (int i = 0; i < npnts; ++i)
		P[i] = pnts[i];
	std::sort(P.begin(), P.end(), ChainSortPred());
	
	const int N = npnts;
	int bot = 0;	// Indices for bottom and top of the stack.
	int top = -1;
	int i;			// Array scan index.

	// Get the indices of points with min x-coord and min|max y-coord.
	int minmin = 0;
	int minmax;
	
	float xmin = P[0].x;
	for (i = 1; i < N; ++i)
	{
		if (P[i].x > xmin)
			break;
	}
	
	minmax = i - 1;
	if (minmax == N - 1)
	{
		// Degenerate case: all x-coords == xmin.
		hull.p[++top] = P[minmin];
		if (P[minmax].y != P[minmin].y)		// A nontrivial segment.
			hull.p[++top] = P[minmax];
		hull.p[++top] = P[minmin];			// Add polygon endpoint.
		hull.p.resize(top);
		return hull;
	}

	// Get the indices of points with max x-coord and min|max y-coord.
	int maxmin;
	int maxmax = N - 1;
	
	float xmax = P[N - 1].x;
	for (i = N - 2; i >= 0; --i)
	{
		if (P[i].x < xmax)
			break;
	}
	
	maxmin = i + 1;

	// Compute the lower hull on the stack H.
	hull.p[++top] = P[minmin];      // Push minmin point onto stack.
	i = minmax;
	while (++i <= maxmin)
	{
		// The lower line joins P[minmin] with P[maxmin].
		if (isLeft(P[minmin], P[maxmin], P[i]) >= 0.0f && i < maxmin)
			continue;	// Ignore P[i] above or on the lower line

		while (top > 0)		// There are at least 2 points on the stack.
		{
			// Test if P[i] is left of the line at the stack top.
			if (isLeft(hull.p[top - 1], hull.p[top], P[i]) > 0.0f)
				break;		// P[i] is a new hull vertex.
			else
				--top;		// Pop top point off stack.
		}
		hull.p[++top] = P[i];		// Push P[i] onto stack.
	}

	// Next, compute the upper hull on the stack H above the bottom hull.
	if (maxmax != maxmin)			// If distinct xmax points
		hull.p[++top] = P[maxmax];	// 	push maxmax point onto stack.
	bot = top;				// The bottom point of the upper hull stack.
	i = maxmin;
	while (--i >= minmax)
	{
		// The upper line joins P[maxmax] with P[minmax].
		if (isLeft(P[maxmax], P[minmax], P[i]) >= 0.0f && i > minmax)
			continue;	// Ignore P[i] below or on the upper line.

		while (top > bot)	// At least 2 points on the upper stack.
		{
			// Test if P[i] is left of the line at the stack top.
			if (isLeft(hull.p[top - 1], hull.p[top], P[i]) > 0)
				break;		// P[i] is a new hull vertex.
			else
				--top;		// Pop top point off stack.
		}
		hull.p[++top] = P[i];		// Push P[i] onto stack.
	}
	
	if (minmax != minmin)
		hull.p[++top] = P[minmin];	// Push joining endpoint onto stack.

	hull.p.resize(top);
	return hull;
}

Winding2 Winding2::convexHull(const AlignedVector< Vector2 >& pnts)
{
	return convexHull(&pnts[0], int(pnts.size()));
}

}
