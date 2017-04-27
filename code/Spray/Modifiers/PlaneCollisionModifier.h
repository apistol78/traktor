/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_PlaneCollisionModifier_H
#define traktor_spray_PlaneCollisionModifier_H

#include "Core/Math/Plane.h"
#include "Spray/Modifier.h"

namespace traktor
{
	namespace spray
	{

/*! \brief Plane collision modifier.
 * \ingroup Spray
 */
class PlaneCollisionModifier : public Modifier
{
	T_RTTI_CLASS;

public:
	PlaneCollisionModifier(const Plane& plane, float radius, float restitution);

#if defined(T_MODIFIER_USE_PS3_SPURS)
	virtual void update(SpursJobQueue* jobQueue, const Scalar& deltaTime, const Transform& transform, PointVector& points) const T_OVERRIDE T_FINAL;
#else
	virtual void update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const T_OVERRIDE T_FINAL;
#endif

private:
	Plane m_plane;
	Scalar m_radius;
	Scalar m_restitution;
};

	}
}

#endif	// traktor_spray_PlaneCollisionModifier_H
