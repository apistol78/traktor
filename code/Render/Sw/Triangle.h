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

	// Block size, standard 8x8 (must be power of two)
	const int q = 8;

	// Start in corner of 8x8 block
	minx &= ~(q - 1);
	miny &= ~(q - 1);

	// Half-edge constants
	int C1 = DY12 * X1 - DX12 * Y1;
	int C2 = DY23 * X2 - DX23 * Y2;
	int C3 = DY31 * X3 - DX31 * Y3;

	// Correct for fill convention
	if(DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
	if(DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
	if(DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;

	// Loop through blocks
	for(int y = miny; y < maxy; y += q)
	{
		for(int x = minx; x < maxx; x += q)
		{
			// Corners of block
			int x0 = x << 4;
			int x1 = (x + q - 1) << 4;
			int y0 = y << 4;
			int y1 = (y + q - 1) << 4;

			// Evaluate half-space functions
			bool a00 = C1 + DX12 * y0 - DY12 * x0 > 0;
			bool a10 = C1 + DX12 * y0 - DY12 * x1 > 0;
			bool a01 = C1 + DX12 * y1 - DY12 * x0 > 0;
			bool a11 = C1 + DX12 * y1 - DY12 * x1 > 0;
			int a = (a00 << 0) | (a10 << 1) | (a01 << 2) | (a11 << 3);

			bool b00 = C2 + DX23 * y0 - DY23 * x0 > 0;
			bool b10 = C2 + DX23 * y0 - DY23 * x1 > 0;
			bool b01 = C2 + DX23 * y1 - DY23 * x0 > 0;
			bool b11 = C2 + DX23 * y1 - DY23 * x1 > 0;
			int b = (b00 << 0) | (b10 << 1) | (b01 << 2) | (b11 << 3);

			bool c00 = C3 + DX31 * y0 - DY31 * x0 > 0;
			bool c10 = C3 + DX31 * y0 - DY31 * x1 > 0;
			bool c01 = C3 + DX31 * y1 - DY31 * x0 > 0;
			bool c11 = C3 + DX31 * y1 - DY31 * x1 > 0;
			int c = (c00 << 0) | (c10 << 1) | (c01 << 2) | (c11 << 3);

			// Skip block when outside an edge
			if(a == 0x0 || b == 0x0 || c == 0x0)
				continue;

			// Accept whole block when totally covered
			if(a == 0xF && b == 0xF && c == 0xF)
			{
				for(int iy = 0; iy < q; iy++)
					(visitor->*visitMethod)(userData, x, x + q, y + iy);
			}
			else	// Partially covered block
			{
				int CY1 = C1 + DX12 * y0 - DY12 * x0;
				int CY2 = C2 + DX23 * y0 - DY23 * x0;
				int CY3 = C3 + DX31 * y0 - DY31 * x0;

				for(int iy = y; iy < y + q; iy++)
				{
					int CX1 = CY1;
					int CX2 = CY2;
					int CX3 = CY3;

					for(int ix = x; ix < x + q; ix++)
					{
						if(CX1 > 0 && CX2 > 0 && CX3 > 0)
						{
							int ix1 = ix;
							for (++ix; ix < x + q; ix++)
							{
								CX1 -= FDY12;
								CX2 -= FDY23;
								CX3 -= FDY31;

								if(!(CX1 > 0 && CX2 > 0 && CX3 > 0))
									break;
							}

							(visitor->*visitMethod)(userData, ix1, ix, iy);
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
		}
	}
}

//@}

	}
}

#endif	// traktor_render_Triangle_H
