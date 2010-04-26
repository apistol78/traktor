#ifndef traktor_Frustum_H
#define traktor_Frustum_H

#include "Core/Config.h"
#include "Core/Containers/StaticVector.h"
#include "Core/Math/MathConfig.h"
#include "Core/Math/Const.h"
#include "Core/Math/Plane.h"
#include "Core/Math/Aabb.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Frustum
 * \ingroup Core
 */
class T_DLLCLASS Frustum
{
public:
	enum PlaneSide
	{
		PsLeft,
		PsRight,
		PsBottom,
		PsTop,
		PsNear,
		PsFar
	};

	StaticVector< Plane, 12 > planes;
	Vector4 corners[8];
	Vector4 center;

	Frustum();

	void buildPerspective(float vfov, float aspect, float zn, float zf);

	void buildOrtho(float width, float height, float zn, float zf);

	void setNearZ(const Scalar& zn);

	Scalar getNearZ() const;

	void setFarZ(const Scalar& zf);

	Scalar getFarZ() const;

	bool inside(const Vector4& point) const;
	
	bool inside(const Vector4& center, const Scalar& radius) const;
	
	bool inside(const Aabb& aabb) const;

private:
	void update();
};

}

#endif	// traktor_Frustum_H
