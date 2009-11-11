#ifndef traktor_animation_AnimatedMeshEntityData_H
#define traktor_animation_AnimatedMeshEntityData_H

#include "Resource/Proxy.h"
#include "World/Entity/SpatialEntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class SkinnedMesh;

	}

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

class AnimatedMeshEntity;
class Skeleton;
class IPoseControllerData;

/*! \brief Animated mesh entity data.
 * \ingroup Animation
 */
class T_DLLCLASS AnimatedMeshEntityData : public world::SpatialEntityData
{
	T_RTTI_CLASS(AnimatedMeshEntityData)

public:
	Ref< AnimatedMeshEntity > createEntity(resource::IResourceManager* resourceManager, physics::PhysicsManager* physicsManager) const;

	virtual bool serialize(Serializer& s);

	inline const resource::Proxy< mesh::SkinnedMesh >& getMesh() const { return m_mesh; }

	inline const resource::Proxy< Skeleton >& getSkeleton() const { return m_skeleton; }

	inline const Ref< IPoseControllerData >& getPoseControllerData() const { return m_poseController; }

private:
	mutable resource::Proxy< mesh::SkinnedMesh > m_mesh;
	mutable resource::Proxy< Skeleton > m_skeleton;
	Ref< IPoseControllerData > m_poseController;
};

	}
}

#endif	// traktor_animation_AnimatedMeshEntityData_H
