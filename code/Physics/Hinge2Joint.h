/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_Hinge2Joint_H
#define traktor_physics_Hinge2Joint_H

#include "Physics/Joint.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*! \brief 2 axis hinge joint.
 * \ingroup Physics
 */
class T_DLLCLASS Hinge2Joint : public Joint
{
	T_RTTI_CLASS;

public:
	virtual void addTorques(float torqueAxis1, float torqueAxis2) = 0;

	virtual float getAngleAxis1() const = 0;

	virtual void setVelocityAxis1(float velocityAxis1) = 0;

	virtual void setVelocityAxis2(float velocityAxis2) = 0;

	virtual void getAnchors(Vector4& outAnchor1, Vector4& outAnchor2) const = 0;
};

	}
}

#endif	// traktor_physics_Hinge2Joint_H
