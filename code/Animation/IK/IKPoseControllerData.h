/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_animation_IKPoseControllerData_H
#define traktor_animation_IKPoseControllerData_H

#include "Animation/IPoseControllerData.h"

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

/*! \brief IK pose evaluation controller data.
 * \ingroup Animation
 */
class T_DLLCLASS IKPoseControllerData : public IPoseControllerData
{
	T_RTTI_CLASS;

public:
	IKPoseControllerData();

	virtual Ref< IPoseController > createInstance(
		resource::IResourceManager* resourceManager,
		physics::PhysicsManager* physicsManager,
		const Skeleton* skeleton,
		const Transform& worldTransform
	) T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	IPoseControllerData* getNeutralPoseController() const { return m_neutralPoseController; }

private:
	Ref< IPoseControllerData > m_neutralPoseController;
	uint32_t m_solverIterations;
};

	}
}

#endif	// traktor_animation_IKPoseControllerData_H
