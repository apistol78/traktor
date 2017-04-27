/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_Triangle_H
#define traktor_render_Triangle_H

#include <cmath>
#include "Core/Math/Vector2.h"

namespace traktor
{
	namespace render
	{

/*! \ingroup SW */
//@{

		namespace
		{

inline int _iround(float f)
{
	return int(f);
}

inline int _min(int a, int b, int c)
{
	return (a < b) ? ((a < c) ? a : c) : ((b < c) ? b : c);
}

inline int _max(int a, int b, int c)
{
	return (a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c);
}

		}

template < typename VisitorType, typename UserDataType >
void triangle(
	const Vector2* v,
	VisitorType* visitor,
	void (VisitorType::*visitMethod)(const UserDataType& userData, int x1, int x2, int y),
	const UserDataType& userData
)
{
	// 28.4 fixed-point coordinates
	const int Y1 = _iround(16.0f * v[0].y);
	const int Y2 = _iround(16.0f * v[1].y);
	const int Y3 = _iround(16.0f * v[2].y);

	const int X1 = _iround(16.0f * v[0].x);
	const int X2 = _iround(16.0f * v[1].x);
	const int X3 = _iround(16.0f * v[2].x);

	// Deltas
	const int DX12 = X1 - X2;
	const int DX23 = X2 - X3;
	const int DX31 = X3 - X1;

	const int DY12 = Y1 - Y2;
	const int DY23 = Y2 - Y3;
	const int DY31 = Y3 - Y1;

	// Fixed-point deltas
	const int FDX12 = DX12 << 4;
	const int FDX23 = DX23 << 4;
	const int FDX31 = DX31 << 4;

	const int FDY12 = DY12 << 4;
	const int FDY23 = DY23 << 4;
	const int FDY31 = DY31 << 4;

	// Bounding rectangle
	int minx = (_min(X1, X2, X3) + 0xF) >> 4;
	int maxx = (_max(X1, X2, X3) + 0xF) >> 4;
	int miny = (_min(Y1, Y2, Y3) + 0xF) >> 4;
	int maxy = (_max(Y1, Y2, Y3) + 0xF) >> 4;

	// Half-edge constants
	int C1 = DY12 * X1 - DX12 * Y1;
	int C2 = DY23 * X2 - DX23 * Y2;
	int C3 = DY31 * X3 - DX31 * Y3;

	// Correct for fill convention
	if(DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
	if(DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
	if(DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;

	int CY1 = C1 + DX12 * (miny << 4) - DY12 * (minx << 4);
	int CY2 = C2 + DX23 * (miny << 4) - DY23 * (minx << 4);
	int CY3 = C3 + DX31 * (miny << 4) - DY31 * (minx << 4);

	for(int y = miny; y < maxy; y++)
	{
		int CX1 = CY1;
		int CX2 = CY2;
		int CX3 = CY3;

		for(int x = minx; x < maxx; x++)
		{
			if(CX1 > 0 && CX2 > 0 && CX3 > 0)
			{
				int x1 = x;
				for (++x; x < maxx; x++)
				{
					CX1 -= FDY12;
					CX2 -= FDY23;
					CX3 -= FDY31;

					if(!(CX1 > 0 && CX2 > 0 && CX3 > 0))
						break;
				}

				(visitor->*visitMethod)(userData, x1, x, y);
			}

			CX1 -= FDY12;
			CX2 -= FDY23;
			CX3 -= FDY31;
		}

		CY1 += FDX12;
		CY2 += FDX23;
		CY3 += FDX31;
	}
}

//@}

	}
}

#endif	// traktor_render_Triangle_H
