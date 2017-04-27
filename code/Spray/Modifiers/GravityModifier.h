/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_GravityModifier_H
#define traktor_spray_GravityModifier_H

#include "Spray/Modifier.h"

namespace traktor
{
	namespace spray
	{

/*! \brief Gravity modifier.
 * \ingroup Spray
 */
class GravityModifier : public Modifier
{
	T_RTTI_CLASS;

public:
	GravityModifier(const Vector4& gravity, bool world);

#if defined(T_MODIFIER_USE_PS3_SPURS)
	virtual void update(SpursJobQueue* jobQueue, const Scalar& deltaTime, const Transform& transform, PointVector& points) const T_OVERRIDE T_FINAL;
#else
	virtual void update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const T_OVERRIDE T_FINAL;
#endif

private:
	Vector4 m_gravity;
	bool m_world;
};

	}
}

#endif	// traktor_spray_GravityModifier_H
