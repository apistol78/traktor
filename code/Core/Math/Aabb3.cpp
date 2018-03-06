/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include <limits>
#include "Core/Math/Aabb3.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace
	{

bool intersectSegment1D(float smin, float smax, float raystart, float rayend, float& outEnter, float& outExit)
{
	float raydir = rayend - raystart;

	if (abs< float >(raydir) <= FUZZY_EPSILON)
	{
		if (raystart < smin || raystart > smax)
			return false;
		else
			return true;
	}

	float enter = (smin - raystart) / raydir;
	float exit = (smax - raystart) / raydir;

	if (enter > exit)
		std::swap< float >(enter, exit);

	if (outEnter > exit || enter > outExit)
		return false;

	outEnter = max< float >(outEnter, enter);
	outExit = min< float >(outExit, exit);
	return true;
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

	float T_MATH_ALIGN16 mnmx[2][4];
	mn.storeAligned(mnmx[0]);
	mx.storeAligned(mnmx[1]);

	float T_MATH_ALIGN16 pe[4];
	p.storeAligned(pe);

	float T_MATH_ALIGN16 ide[4];
	id.storeAligned(ide);

	int32_t sign[] =
	{
		ide[0] < 0.0f,
		ide[1] < 0.0f,
		ide[2] < 0.0f
	};

	float tmin = (mnmx[sign[0]][0] - pe[0]) * ide[0];
	float tmax = (mnmx[1 - sign[0]][0] - pe[0]) * ide[0];
	float tymin = (mnmx[sign[1]][1] - pe[1]) * ide[1];
	float tymax = (mnmx[1 - sign[1]][1] - pe[1]) * ide[1];
	if ((tmin > tymax) || (tymin > tmax))
		return false;

	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;

	float tzmin = (mnmx[sign[2]][2] - pe[2]) * ide[2];
	float tzmax = (mnmx[1-sign[2]][2] - pe[2]) * ide[2];
	if ((tmin > tzmax) || (tzmin > tmax))
		return false;

	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;

	outDistanceEnter = Scalar(tmin);
	outDistanceExit = Scalar(tmax);

	return true;
}

bool Aabb3::intersectSegment(const Vector4& p1, const Vector4& p2, Scalar& outDistance) const
{
	float enter = 0.0f, exit = 1.0f;

	if (!intersectSegment1D(mn.x(), mx.x(), p1.x(), p2.x(), enter, exit))
		return false;
	if (!intersectSegment1D(mn.y(), mx.y(), p1.y(), p2.y(), enter, exit))
		return false;
	if (!intersectSegment1D(mn.z(), mx.z(), p1.z(), p2.z(), enter, exit))
		return false;

	outDistance = Scalar(enter);
	return true;
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
