#include <limits>
#include "Core/Math/Aabb3.h"

namespace traktor
{
	namespace
	{

T_FORCE_INLINE uint32_t outcode(const Vector4& p, const Vector4& halfExtent) 
{
	return
		(p.x() < -halfExtent.x() ? 0x01 : 0x00) |    
		(p.y() < -halfExtent.y() ? 0x02 : 0x00) |
		(p.z() < -halfExtent.z() ? 0x04 : 0x00) |
		(p.x() >  halfExtent.x() ? 0x08 : 0x00) |
		(p.y() >  halfExtent.y() ? 0x10 : 0x00) |
		(p.z() >  halfExtent.z() ? 0x20 : 0x00);
}

	}

Aabb3::Aabb3()
:	mn(std::numeric_limits< float >::max(), std::numeric_limits< float >::max(), std::numeric_limits< float >::max())
,	mx(-std::numeric_limits< float >::max(), -std::numeric_limits< float >::max(), -std::numeric_limits< float >::max())
{
}

Aabb3::Aabb3(const Aabb3& aabb)
:	mn(aabb.mn)
,	mx(aabb.mx)
{
}

Aabb3::Aabb3(const Vector4& mn_, const Vector4& mx_)
:	mn(mn_)
,	mx(mx_)
{
}

void Aabb3::getExtents(Vector4 extents[8]) const
{
	extents[0] = mn.xyz1();
	extents[1] = Vector4(mn.x(), mx.y(), mn.z(), 1.0f);
	extents[2] = Vector4(mx.x(), mx.y(), mn.z(), 1.0f);
	extents[3] = Vector4(mx.x(), mn.y(), mn.z(), 1.0f);
	extents[4] = Vector4(mn.x(), mn.y(), mx.z(), 1.0f);
	extents[5] = Vector4(mn.x(), mx.y(), mx.z(), 1.0f);
	extents[6] = mx.xyz1();
	extents[7] = Vector4(mx.x(), mn.y(), mx.z(), 1.0f);
}

bool Aabb3::inside(const Vector4& pt) const
{
	return
		compareAllGreaterEqual(pt.xyz1(), mn.xyz0()) &&
		compareAllLessEqual(pt.xyz0(), mx.xyz1());
}

bool Aabb3::outside(const Vector4& pt) const
{
	return !inside(pt);
}

bool Aabb3::surface(const Vector4& pt, float margin) const
{
	Vector4 x = (mx - mn).normalized() * Scalar(margin);
	if (
		compareAllGreaterEqual(pt.xyz1(), (mn - x).xyz0()) &&
		compareAllLessEqual(pt.xyz0(), (mx + x).xyz1())
	)
	{
		// Point inside box+margin.
		if (
			compareAllGreaterEqual(pt.xyz1(), (mn + x).xyz0()) &&
			compareAllLessEqual(pt.xyz0(), (mx - x).xyz1())
		)
		{
			// Point inside box-margin.
			return false;
		}
		return true;
	}
	else
	{
		// Point outside box+margin.
		return false;
	}
}

bool Aabb3::intersectRay(const Vector4& p, const Vector4& d, Scalar& outDistance) const
{
	Scalar distanceExitDummy;
	return intersectRay(p, d, outDistance, distanceExitDummy);
}

bool Aabb3::intersectRay(const Vector4& p, const Vector4& d, Scalar& outDistanceEnter, Scalar& outDistanceExit) const
{
	Vector4 id = Scalar(1.0f) / d.xyz1();
	int32_t sign[] =
	{
		id.x() < 0.0f,
		id.y() < 0.0f,
		id.z() < 0.0f
	};

	const Vector4 mnmx[] = { mn, mx };

	Scalar tmin = (mnmx[sign[0]].x() - p.x()) * id.x();
	Scalar tmax = (mnmx[1 - sign[0]].x() - p.x()) * id.x();
	Scalar tymin = (mnmx[sign[1]].y() - p.y()) * id.y();
	Scalar tymax = (mnmx[1 - sign[1]].y() - p.y()) * id.y();
	if ((tmin > tymax) || (tymin > tmax))
		return false;

	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;

	Scalar tzmin = (mnmx[sign[2]].z() - p.z()) * id.z();
	Scalar tzmax = (mnmx[1-sign[2]].z() - p.z()) * id.z();
	if ((tmin > tzmax) || (tzmin > tmax))
		return false;

	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;

	outDistanceEnter = tmin;
	outDistanceExit = tmax;

	return true;
}

bool Aabb3::intersectSegment(const Vector4& p1, const Vector4& p2, Scalar& outDistance) const
{
	Vector4 start = p1 - getCenter();
	Vector4 end = p2 - getCenter();

	Vector4 halfExtent = getExtent();

	uint32_t startCode = outcode(start, halfExtent);
	uint32_t endCode = outcode(end, halfExtent);

	if ((startCode & endCode) != 0x00)
		return false;

	Vector4 r = end - start;

	Scalar lambdaStart = Scalar(0.0f);
	Scalar lambdaEnd = Scalar(1.0f);

	uint32_t bit = 1;
	Scalar sign = Scalar(1.0f);

	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			Scalar lambda = (-start[j] - halfExtent[j] * sign) / r[j];
			if (startCode & bit)
			{
				if (lambdaStart <= lambda)
					lambdaStart = lambda;
			}
			else if (endCode & bit)
			{
				if (lambda < lambdaEnd)
					lambdaEnd = lambda;
			}
			bit <<= 1;
		}
		sign = Scalar(-1.0f);
	}

	if (lambdaStart <= lambdaEnd)
	{
		outDistance = ((startCode != 0x00) ? lambdaStart : lambdaEnd) * r.length();
		return true;
	}

	return false;
}

Aabb3 Aabb3::transform(const Matrix44& m) const
{
	Vector4 c = m * getCenter();

	Vector4 e = getExtent();
	Vector4 x = m.axisX() * e.x();
	Vector4 y = m.axisY() * e.y();
	Vector4 z = m.axisZ() * e.z();

	Vector4 first = c + x + y + z;
	Aabb3 aabb(first, first);

	aabb.contain(c - x + y + z);
	aabb.contain(c + x - y + z);
	aabb.contain(c - x - y + z);
	aabb.contain(c + x + y - z);
	aabb.contain(c - x + y - z);
	aabb.contain(c + x - y - z);
	aabb.contain(c - x - y - z);

	return aabb;
}

Aabb3 Aabb3::transform(const Transform& tf) const
{
	Vector4 c = tf * getCenter();

	Vector4 ax = tf.rotation() * Vector4(1.0f, 0.0f, 0.0f);
	Vector4 ay = tf.rotation() * Vector4(0.0f, 1.0f, 0.0f);
	Vector4 az = tf.rotation() * Vector4(0.0f, 0.0f, 1.0f);

	Vector4 e = getExtent();
	Vector4 x = ax * e.x();
	Vector4 y = ay * e.y();
	Vector4 z = az * e.z();

	Vector4 first = c + x + y + z;
	Aabb3 aabb(first, first);

	aabb.contain(c - x + y + z);
	aabb.contain(c + x - y + z);
	aabb.contain(c - x - y + z);
	aabb.contain(c + x + y - z);
	aabb.contain(c - x + y - z);
	aabb.contain(c + x - y - z);
	aabb.contain(c - x - y - z);

	return aabb;
}

const int* Aabb3::getFaces()
{
	static const int s_faces[] =
	{
		0, 1, 2, 3,
		3, 2, 6, 7,
		7, 6, 5, 4,
		4, 5, 1, 0,
		1, 5, 6, 2,
		0, 3, 7, 4
	};
	return s_faces;
}

const int* Aabb3::getEdges()
{
	static const int s_edges[] =
	{
		0, 1, 
		1, 2,
		2, 3, 
		3, 0,
		7, 6,
		6, 5,
		5, 4,
		4, 7,
		4, 0,
		5, 1,
		6, 2,
		7, 3
	};
	return s_edges;
}

const int* Aabb3::getEdgeAdjacency()
{
	static const int s_edgeAdjacency[] =
	{
		0, 3,
		0, 4,
		0, 1,
		0, 5,
		2, 1,
		2, 4,
		2, 3,
		2, 5,
		5, 3,
		3, 4,
		4, 1,
		1, 5
	};
	return s_edgeAdjacency;
}

const Vector4* Aabb3::getNormals()
{
	static const Vector4 s_normals[] =
	{
		Vector4( 0.0f,  0.0f, -1.0f),
		Vector4( 1.0f,  0.0f,  0.0f),
		Vector4( 0.0f,  0.0f,  1.0f),
		Vector4(-1.0f,  0.0f,  0.0f),
		Vector4( 0.0f,  1.0f,  0.0f),
		Vector4( 0.0f, -1.0f,  0.0f)
	};
	return s_normals;
}

}
