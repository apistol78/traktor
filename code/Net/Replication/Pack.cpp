#include "Core/Math/Const.h"
#include "Net/Replication/Pack.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

#define SIGN_MASK  0xe000	// upper 3 bits
#define XSIGN_MASK 0x8000
#define YSIGN_MASK 0x4000
#define ZSIGN_MASK 0x2000
#define TOP_MASK  0x1f80	// middle 6 bits - xbits
#define BOTTOM_MASK  0x007f	// lower 7 bits - ybits

bool s_initialized = false;
float s_uvAdjustment[0x2000];
Vector4 s_tmpVec;

void initializeTables()
{
	if (s_initialized)
		return;

	for (int idx = 0; idx < 0x2000; ++idx)
	{
		long xbits = idx >> 7;
		long ybits = idx & BOTTOM_MASK;
          
		// Map the numbers back to the triangle (0,0)-(0,127)-(127,0).
		if ((xbits + ybits) >= 127) 
		{ 
			xbits = 127 - xbits; 
			ybits = 127 - ybits; 
		}
 
		// Convert to 3D vectors.
		float x = (float)xbits;
		float y = (float)ybits;
		float z = (float)(126 - xbits - ybits);
                 
		// Calculate the amount of normalization required.
		s_uvAdjustment[idx] = 1.0f / sqrtf( y*y + z*z + x*x );
	}

	s_initialized = true;
}

		}

PackedUnitVector::PackedUnitVector()
:	m_v(0)
{
	initializeTables();
}

PackedUnitVector::PackedUnitVector(const Vector4& v)
:	m_v(0)
{
	initializeTables();
	pack(v);
}

PackedUnitVector::PackedUnitVector(uint16_t v)
:	m_v(v)
{
	initializeTables();
}

void PackedUnitVector::pack(const Vector4& v)
{
	m_v = 0;
 
	if (v.length2() < FUZZY_EPSILON * FUZZY_EPSILON)
		return;

	float T_MATH_ALIGN16 e[4];
	v.storeAligned(e);

	if ( e[0] < 0 ) { m_v |= XSIGN_MASK; e[0] = -e[0]; }
	if ( e[1] < 0 ) { m_v |= YSIGN_MASK; e[1] = -e[1]; }
	if ( e[2] < 0 ) { m_v |= ZSIGN_MASK; e[2] = -e[2]; }
 
	// project the normal onto the plane that goes through
	// X0=(1,0,0),Y0=(0,1,0),Z0=(0,0,1).
 
	// on that plane we choose an (projective!) coordinate system
	// such that X0->(0,0), Y0->(126,0), Z0->(0,126),(0,0,0)->Infinity
 
	// a little slower... old pack was 4 multiplies and 2 adds. 
	// This is 2 multiplies, 2 adds, and a divide....
	float w = 126.0f / (e[0] + e[1] + e[2]);
	long xbits = (long)(e[0] * w);
	long ybits = (long)(e[1] * w);
 
	T_ASSERT( xbits <  127 );
	T_ASSERT( xbits >= 0   );
	T_ASSERT( ybits <  127 );
	T_ASSERT( ybits >= 0   );
 
	// Now we can be sure that 0<=xp<=126, 0<=yp<=126, 0<=xp+yp<=126
 
	// However for the sampling we want to transform this triangle 
	// into a rectangle.
	if (xbits >= 64)
	{ 
		xbits = 127 - xbits; 
		ybits = 127 - ybits; 
	}
 
	// now we that have xp in the range (0,127) and yp in the 
	// range (0,63), we can pack all the bits together
	m_v |= (xbits << 7);
	m_v |= ybits;
}

Vector4 PackedUnitVector::unpack() const
{
	float T_MATH_ALIGN16 e[4];

	// if we do a straightforward backward transform
	// we will get points on the plane X0,Y0,Z0
	// however we need points on a sphere that goes through 
	// these points.
	// therefore we need to adjust x,y,z so that x^2+y^2+z^2=1
 
	// by normalizing the vector. We have already precalculated 
	// the amount by which we need to scale, so all we do is a 
	// table lookup and a multiplication
 
	// get the x and y bits
	long xbits = ((m_v & TOP_MASK) >> 7);
	long ybits = (m_v & BOTTOM_MASK);
 
	// map the numbers back to the triangle (0,0)-(0,126)-(126,0)
	if ((xbits + ybits) >= 127)
	{ 
		xbits = 127 - xbits; 
		ybits = 127 - ybits; 
	}
 
	// do the inverse transform and normalization
	// costs 3 extra multiplies and 2 subtracts. No big deal.         
	float uvadj = s_uvAdjustment[m_v & ~SIGN_MASK];
	e[0] = uvadj * (float)xbits;
	e[1] = uvadj * (float)ybits;
	e[2] = uvadj * (float)(126 - xbits - ybits);
 
	// set all the sign bits
	if (m_v & XSIGN_MASK) e[0] = -e[0];
	if (m_v & YSIGN_MASK) e[1] = -e[1];
	if (m_v & ZSIGN_MASK) e[2] = -e[2];
 
	return Vector4::loadAligned(e);
}

	}
}
