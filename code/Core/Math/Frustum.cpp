#include <cmath>
#include "Core/Math/Frustum.h"

namespace traktor
{

Frustum::Frustum()
:	planes(6)
{
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

bool Frustum::inside(const Vector4& point) const
{
	const Scalar c_zero(0.0f);

	for (uint32_t i = 0; i < planes.size(); ++i)
	{
		if (planes[i].distance(point) < c_zero)
			return false;
	}

	return true;
}

bool Frustum::inside(const Vector4& center_, const Scalar& radius) const
{
	for (uint32_t i = 0; i < planes.size(); ++i)
	{
		if (planes[i].distance(center_) < -radius)
			return false;
	}
	return true;
}

bool Frustum::inside(const Aabb& aabb) const
{
	const Scalar c_zero(0.0f);
	for (uint32_t i = 0; i < planes.size(); ++i)
	{
		Vector4 n(
			select(planes[i].normal().x(), aabb.mx.x(), aabb.mn.x()),
			select(planes[i].normal().y(), aabb.mx.y(), aabb.mn.y()),
			select(planes[i].normal().z(), aabb.mx.z(), aabb.mn.z())
		);
		Vector4 p(
			select(planes[i].normal().x(), aabb.mn.x(), aabb.mx.x()),
			select(planes[i].normal().y(), aabb.mn.y(), aabb.mx.y()),
			select(planes[i].normal().z(), aabb.mn.z(), aabb.mx.z())
		);

		if (planes[i].distance(n) > c_zero)	// outside
			return false;

		if (planes[i].distance(p) > c_zero)	// intersecting
			return true;
	}
	return true;	// totally inside.
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
