#ifndef traktor_animation_AnimatedMeshEntityData_H
#define traktor_animation_AnimatedMeshEntityData_H

#include "Resource/Id.h"
#include "World/EntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class SkinnedMesh;

	}

	namespace physics
	{

class PhysicsManager;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace animation
	{

class AnimatedMeshEntity;
class Skeleton;
class IPoseControllerData;

/*! \brief Animated mesh entity data.
 * \ingroup Animation
 */
class T_DLLCLASS AnimatedMeshEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	AnimatedMeshEntityData();

	Ref< AnimatedMeshEntity > createEntity(resource::IResourceManager* resourceManager, physics::PhysicsManager* physicsManager) const;

	virtual void serialize(ISerializer& s);

	const resource::Id< mesh::SkinnedMesh >& getMesh() const { return m_mesh; }

	const resource::Id< Skeleton >& getSkeleton() const { return m_skeleton; }

	IPoseControllerData* getPoseControllerData() const { return m_poseController; }

private:
	resource::Id< mesh::SkinnedMesh > m_mesh;
	resource::Id< Skeleton > m_skeleton;
	Ref< IPoseControllerData > m_poseController;
	bool m_normalizePose;
	bool m_normalizeTransform;
	bool m_screenSpaceCulling;
};

	}
}

#endif	// traktor_animation_AnimatedMeshEntityData_H
