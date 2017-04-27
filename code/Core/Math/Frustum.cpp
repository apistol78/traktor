/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cmath>
#include "Core/Math/Frustum.h"

namespace traktor
{
	namespace
	{

const static Scalar c_scalarZero(0.0f);

	}

Frustum::Frustum()
:	planes(6)
{
}

void Frustum::buildFromPlanes(const Plane planes_[6])
{
	for (int32_t i = 0; i < 6; ++i)
		planes[i] = planes_[i];
	update();
}

void Frustum::buildPerspective(float vfov, float aspect, float zn, float zf)
{
	vfov /= 2.0f;

	float a = sinf(vfov) * aspect;
	float b = cosf(vfov);
	float c = sqrtf(a * a + b * b);
	float hfov = asinf(a / c);

	planes[0].set(Vector4(cosf(hfov), 0.0f, sinf(hfov)), Scalar(0.0f));
	planes[1].set(Vector4(-cosf(hfov), 0.0f, sinf(hfov)), Scalar(0.0f));
	planes[2].set(Vector4(0.0f, cosf(vfov), sinf(vfov)), Scalar(0.0f));
	planes[3].set(Vector4(0.0f, -cosf(vfov), sinf(vfov)), Scalar(0.0f));
	planes[4].set(Vector4(0.0f, 0.0f,  1.0f), Scalar(zn));
	planes[5].set(Vector4(0.0f, 0.0f, -1.0f), Scalar(-zf));

	update();
}

void Frustum::buildOrtho(float width, float height, float zn, float zf)
{
	Scalar hw = Scalar(-width * 0.5f);
	Scalar hh = Scalar(-height * 0.5f);

	planes[0].set(Vector4(1.0f, 0.0f, 0.0f), hw);
	planes[1].set(Vector4(-1.0f, 0.0f, 0.0f), hw);
	planes[2].set(Vector4(0.0f, 1.0f, 0.0f), hh);
	planes[3].set(Vector4(0.0f, -1.0f, 0.0f), hh);
	planes[4].set(Vector4(0.0f, 0.0f, 1.0f), Scalar(zn));
	planes[5].set(Vector4(0.0f, 0.0f, -1.0f), Scalar(-zf));

	update();
}

void Frustum::setNearZ(const Scalar& zn)
{
	planes[PsNear].setDistance(zn);
	update();
}

Scalar Frustum::getNearZ() const
{
	return planes[PsNear].distance();
}

void Frustum::setFarZ(const Scalar& zf)
{
	planes[PsFar].setDistance(-zf);
	update();
}

Scalar Frustum::getFarZ() const
{
	return -planes[PsFar].distance();
}

Frustum::InsideResult Frustum::inside(const Vector4& point) const
{
	for (uint32_t i = 0; i < planes.size(); ++i)
	{
		if (planes[i].distance(point) < c_scalarZero)
			return IrOutside;
	}
	return IrInside;
}

Frustum::InsideResult Frustum::inside(const Vector4& center_, const Scalar& radius) const
{
	bool partial = false;
	Scalar nradius = -radius;
	for (uint32_t i = 0; i < planes.size(); ++i)
	{
		Scalar distance = planes[i].distance(center_);
		if (distance < nradius)
			return IrOutside;
		if (distance < radius)
			partial |= true;
	}
	return partial ? IrPartial : IrInside;
}

Frustum::InsideResult Frustum::inside(const Aabb3& aabb) const
{
	bool partial = false;
	for (uint32_t i = 0; i < planes.size(); ++i)
	{
		Vector4 n = select(planes[i].normal(), aabb.mn, aabb.mx);
		if (planes[i].distance(n) < c_scalarZero)	// outside
			return IrOutside;

		Vector4 p = select(planes[i].normal(), aabb.mx, aabb.mn);
		if (planes[i].distance(p) < c_scalarZero)	// intersecting
			partial |= true;
	}
	return partial ? IrPartial : IrInside;
}

void Frustum::update()
{
	Plane::uniqueIntersectionPoint(planes[PsNear], planes[PsLeft]  , planes[PsTop]   , corners[0]);
	Plane::uniqueIntersectionPoint(planes[PsNear], planes[PsTop]   , planes[PsRight] , corners[1]);
	Plane::uniqueIntersectionPoint(planes[PsNear], planes[PsRight] , planes[PsBottom], corners[2]);
	Plane::uniqueIntersectionPoint(planes[PsNear], planes[PsBottom], planes[PsLeft]  , corners[3]);
	Plane::uniqueIntersectionPoint(planes[PsFar] , planes[PsLeft]  , planes[PsTop]   , corners[4]);
	Plane::uniqueIntersectionPoint(planes[PsFar] , planes[PsTop]   , planes[PsRight] , corners[5]);
	Plane::uniqueIntersectionPoint(planes[PsFar] , planes[PsRight] , planes[PsBottom], corners[6]);
	Plane::uniqueIntersectionPoint(planes[PsFar] , planes[PsBottom], planes[PsLeft]  , corners[7]);

	center = corners[0];
	for (int i = 1; i < 8; ++i)
		center += corners[i];
	center /= Scalar(8.0f);
}

}
