#pragma once

#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

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

class EntityData;
class IEntityBuilder;

	}

	namespace animation
	{

class AnimatedMeshComponent;
class Skeleton;
class IPoseControllerData;

/*! Animated mesh entity data.
 * \ingroup Animation
 */
class T_DLLCLASS AnimatedMeshComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	struct Binding
	{
		std::wstring jointName;
		Ref< const world::EntityData > entityData;

		void serialize(ISerializer& s);
	};

	AnimatedMeshComponentData();

	AnimatedMeshComponentData(
		const resource::Id< mesh::SkinnedMesh >& mesh,
		const resource::Id< Skeleton >& skeleton,
		const IPoseControllerData* poseController
	);

	Ref< AnimatedMeshComponent > createComponent(resource::IResourceManager* resourceManager, physics::PhysicsManager* physicsManager, const world::IEntityBuilder* entityBuilder) const;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< mesh::SkinnedMesh >& getMesh() const { return m_mesh; }

	const resource::Id< Skeleton >& getSkeleton() const { return m_skeleton; }

	const IPoseControllerData* getPoseControllerData() const { return m_poseController; }

	const AlignedVector< Binding >& getBindings() const { return m_bindings; }

private:
	resource::Id< mesh::SkinnedMesh > m_mesh;
	resource::Id< Skeleton > m_skeleton;
	Ref< const IPoseControllerData > m_poseController;
	bool m_normalizePose;
	bool m_normalizeTransform;
	bool m_screenSpaceCulling;
	AlignedVector< Binding > m_bindings;
};

	}
}
