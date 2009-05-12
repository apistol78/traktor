#include <cmath>
#include "Core/Math/Frustum.h"

namespace traktor
{

void Frustum::buildPerspective(float vfov, float aspect, float zn, float zf)
{
	vfov /= 2.0f;

	float a = sinf(vfov) * aspect;
	float b = cosf(vfov);
	float c = sqrtf(a * a + b * b);
	float hfov = asinf(a / c);

	pl[0].set(Vector4(cosf(hfov), 0.0f, sinf(hfov)), Scalar(0.0f));
	pl[1].set(Vector4(-cosf(hfov), 0.0f, sinf(hfov)), Scalar(0.0f));
	pl[2].set(Vector4(0.0f, cosf(vfov), sinf(vfov)), Scalar(0.0f));
	pl[3].set(Vector4(0.0f, -cosf(vfov), sinf(vfov)), Scalar(0.0f));
	pl[4].set(Vector4(0.0f, 0.0f,  1.0f), Scalar(zn));
	pl[5].set(Vector4(0.0f, 0.0f, -1.0f), Scalar(-zf));

	update();
}

void Frustum::buildOrtho(float width, float height, float zn, float zf)
{
	Scalar hw = Scalar(-width * 0.5f);
	Scalar hh = Scalar(-height * 0.5f);

	pl[0].set(Vector4(1.0f, 0.0f, 0.0f), hw);
	pl[1].set(Vector4(-1.0f, 0.0f, 0.0f), hw);
	pl[2].set(Vector4(0.0f, 1.0f, 0.0f), hh);
	pl[3].set(Vector4(0.0f, -1.0f, 0.0f), hh);
	pl[4].set(Vector4(0.0f, 0.0f, 1.0f), Scalar(zn));
	pl[5].set(Vector4(0.0f, 0.0f, -1.0f), Scalar(-zf));

	update();
}

void Frustum::setNearZ(const Scalar& zn)
{
	pl[PsNear].setDistance(zn);
	update();
}

Scalar Frustum::getNearZ() const
{
	return pl[PsNear].distance();
}

void Frustum::setFarZ(const Scalar& zf)
{
	pl[PsFar].setDistance(-zf);
	update();
}

Scalar Frustum::getFarZ() const
{
	return -pl[PsFar].distance();
}

bool Frustum::inside(const Vector4& point) const
{
	const Scalar c_zero(0.0f);

	if (pl[0].distance(point) < c_zero)
		return false;
	if (pl[1].distance(point) < c_zero)
		return false;
	if (pl[2].distance(point) < c_zero)
		return false;
	if (pl[3].distance(point) < c_zero)
		return false;
	if (pl[4].distance(point) < c_zero)
		return false;
	if (pl[5].distance(point) < c_zero)
		return false;

	return true;
}

bool Frustum::inside(const Vector4& center_, const Scalar& radius) const
{
	if (pl[0].distance(center_) < -radius)
		return false;
	if (pl[1].distance(center_) < -radius)
		return false;
	if (pl[2].distance(center_) < -radius)
		return false;
	if (pl[3].distance(center_) < -radius)
		return false;
	if (pl[4].distance(center_) < -radius)
		return false;
	if (pl[5].distance(center_) < -radius)
		return false;

	return true;
}

bool Frustum::inside(const Aabb& aabb) const
{
	const Scalar c_zero(0.0f);
	for (int i = 0; i < 6; ++i)
	{
		Vector4 n(
			select(pl[i].normal().x(), aabb.mx.x(), aabb.mn.x()),
			select(pl[i].normal().y(), aabb.mx.y(), aabb.mn.y()),
			select(pl[i].normal().z(), aabb.mx.z(), aabb.mn.z())
		);
		Vector4 p(
			select(pl[i].normal().x(), aabb.mn.x(), aabb.mx.x()),
			select(pl[i].normal().y(), aabb.mn.y(), aabb.mx.y()),
			select(pl[i].normal().z(), aabb.mn.z(), aabb.mx.z())
		);

		if (pl[i].distance(n) > c_zero)	// outside
			return false;

		if (pl[i].distance(p) > c_zero)	// intersecting
			return true;
	}
	return true;	// totally inside.
}

void Frustum::update()
{
	Plane::uniqueIntersectionPoint(pl[PsNear], pl[PsLeft]  , pl[PsTop]   , corners[0]);
	Plane::uniqueIntersectionPoint(pl[PsNear], pl[PsTop]   , pl[PsRight] , corners[1]);
	Plane::uniqueIntersectionPoint(pl[PsNear], pl[PsRight] , pl[PsBottom], corners[2]);
	Plane::uniqueIntersectionPoint(pl[PsNear], pl[PsBottom], pl[PsLeft]  , corners[3]);
	Plane::uniqueIntersectionPoint(pl[PsFar] , pl[PsLeft]  , pl[PsTop]   , corners[4]);
	Plane::uniqueIntersectionPoint(pl[PsFar] , pl[PsTop]   , pl[PsRight] , corners[5]);
	Plane::uniqueIntersectionPoint(pl[PsFar] , pl[PsRight] , pl[PsBottom], corners[6]);
	Plane::uniqueIntersectionPoint(pl[PsFar] , pl[PsBottom], pl[PsLeft]  , corners[7]);

	center = corners[0];
	for (int i = 1; i < 8; ++i)
		center += corners[i];
	center /= Scalar(8.0f);
}

}
