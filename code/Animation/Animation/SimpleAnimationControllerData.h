/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_animation_SimpleAnimationControllerData_H
#define traktor_animation_SimpleAnimationControllerData_H

#include "Animation/IPoseControllerData.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

class Animation;

/*! \brief
 * \ingroup Animation
 */
class T_DLLCLASS SimpleAnimationControllerData : public IPoseControllerData
{
	T_RTTI_CLASS;

public:
	SimpleAnimationControllerData();

	virtual Ref< IPoseController > createInstance(
		resource::IResourceManager* resourceManager,
		physics::PhysicsManager* physicsManager,
		const Skeleton* skeleton,
		const Transform& worldTransform
	) T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const resource::Id< Animation >& getAnimation() const { return m_animation; }

private:
	resource::Id< Animation > m_animation;
	bool m_linearInterpolation;
};

	}
}

#endif	// traktor_animation_SimpleAnimationControllerData_H
