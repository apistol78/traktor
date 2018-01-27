/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_animation_AnimatedMeshEntityData_H
#define traktor_animation_AnimatedMeshEntityData_H

#include <vector>
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

	namespace world
	{

class IEntityBuilder;

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
	struct Binding
	{
		std::wstring jointName;
		Ref< world::EntityData > entityData;

		void serialize(ISerializer& s);
	};

	AnimatedMeshEntityData();

	Ref< AnimatedMeshEntity > createEntity(resource::IResourceManager* resourceManager, physics::PhysicsManager* physicsManager, const world::IEntityBuilder* entityBuilder) const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const resource::Id< mesh::SkinnedMesh >& getMesh() const { return m_mesh; }

	const resource::Id< Skeleton >& getSkeleton() const { return m_skeleton; }

	IPoseControllerData* getPoseControllerData() const { return m_poseController; }

	const std::vector< Binding >& getBindings() const { return m_bindings; }

private:
	resource::Id< mesh::SkinnedMesh > m_mesh;
	resource::Id< Skeleton > m_skeleton;
	Ref< IPoseControllerData > m_poseController;
	bool m_normalizePose;
	bool m_normalizeTransform;
	bool m_screenSpaceCulling;
	std::vector< Binding > m_bindings;
};

	}
}

#endif	// traktor_animation_AnimatedMeshEntityData_H
