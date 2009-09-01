#ifndef traktor_animation_IPoseControllerData_H
#define traktor_animation_IPoseControllerData_H

#include "Core/Serialization/Serializable.h"
#include "Core/Math/Transform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace physics
	{

class PhysicsManager;

	}

	namespace animation
	{

class IPoseController;
class Skeleton;

/*! \brief Pose evaluation controller data.
 * \ingroup Animation
 */
class T_DLLCLASS IPoseControllerData : public Serializable
{
	T_RTTI_CLASS(IPoseControllerData)

public:
	virtual IPoseController* createInstance(
		resource::IResourceManager* resourceManager,
		physics::PhysicsManager* physicsManager,
		const Skeleton* skeleton,
		const Transform& worldTransform
	) = 0;
};

	}
}

#endif	// traktor_animation_IPoseControllerData_H
