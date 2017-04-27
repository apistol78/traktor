/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_StaticBodyDesc_H
#define traktor_physics_StaticBodyDesc_H

#include "Physics/BodyDesc.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*! \brief Static rigid body description.
 * \ingroup Physics
 */
class T_DLLCLASS StaticBodyDesc : public BodyDesc
{
	T_RTTI_CLASS;

public:
	StaticBodyDesc();

	void setFriction(float friction);

	float getFriction() const;

	void setKinematic(bool kinematic);

	bool isKinematic() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	float m_friction;
	bool m_kinematic;
};

	}
}

#endif	// traktor_physics_StaticBodyDesc_H
