#pragma once

#include "Animation/IPoseController.h"

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

/*! IK pose evaluation controller.
 * \ingroup Animation
 */
class T_DLLCLASS IKPoseController : public IPoseController
{
	T_RTTI_CLASS;

public:
	IKPoseController(IPoseController* poseController, uint32_t solverIterations);

	virtual ~IKPoseController();

	virtual void destroy() override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual bool evaluate(
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& jointTransforms,
		AlignedVector< Transform >& outPoseTransforms,
		bool& outUpdateController
	) override final;

	virtual void estimateVelocities(
		const Skeleton* skeleton,
		AlignedVector< Velocity >& outVelocities
	) override final;

	IPoseController* getNeutralPoseController() const { return m_poseController; }

private:
	Ref< IPoseController > m_poseController;
	uint32_t m_solverIterations;
};

	}
}

