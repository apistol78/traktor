/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/Pose.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Thread/Job.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "Mesh/MeshComponent.h"

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

	namespace render
	{

class Buffer;
class IRenderSystem;

	}

	namespace world
	{

class Entity;

	}

	namespace animation
	{

class Skeleton;
class IPoseController;

/*! Animated mesh entity.
 * \ingroup Animation
 */
class T_DLLCLASS AnimatedMeshComponent : public mesh::MeshComponent
{
	T_RTTI_CLASS;

public:
	struct Binding
	{
		render::handle_t jointHandle;
		Ref< world::Entity > entity;
	};

	AnimatedMeshComponent(
		const Transform& transform,
		const resource::Proxy< mesh::SkinnedMesh >& mesh,
		const resource::Proxy< Skeleton >& skeleton,
		IPoseController* poseController,
		const AlignedVector< int32_t >& jointRemap,
		const AlignedVector< Binding >& bindings,
		render::IRenderSystem* renderSystem,
		bool screenSpaceCulling
	);

	virtual ~AnimatedMeshComponent();

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	virtual void build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass) override final;

	/*! Get base transform of joint. */
	bool getJointTransform(render::handle_t jointName, Transform& outTransform) const;

	/*! Get transform of current pose. */
	bool getPoseTransform(render::handle_t jointName, Transform& outTransform) const;

	/*! Get skin transform of joint in delta space. */
	bool getSkinTransform(render::handle_t jointName, Transform& outTransform) const;

	/*! Set pose transform of joint. */
	bool setPoseTransform(render::handle_t jointName, const Transform& transform, bool inclusive);

	/*! Get skeleton. */
	const resource::Proxy< Skeleton >& getSkeleton() const { return m_skeleton; }

	/*! Set pose evaluation controller. */
	void setPoseController(IPoseController* poseController) { m_poseController = poseController; }

	/*! Get pose evaluation controller. */
	IPoseController* getPoseController() const { return m_poseController; }

	/*! Get all joint entity bindings. */
	const AlignedVector< Binding >& getBindings() const { return m_bindings; }

	/*! Get all joint base transforms. */
	const AlignedVector< Transform >& getJointTransforms() const { return m_jointTransforms; }

	/*! Get all joint pose transforms. */
	const AlignedVector< Transform >& getPoseTransforms() const { return m_poseTransforms; }

private:
	resource::Proxy< mesh::SkinnedMesh > m_mesh;
	resource::Proxy< Skeleton > m_skeleton;
	Ref< IPoseController > m_poseController;
	AlignedVector< int32_t > m_jointRemap;
	AlignedVector< Binding > m_bindings;
	AlignedVector< Transform > m_jointTransforms;
	AlignedVector< Transform > m_poseTransforms;
	AlignedVector< Vector4 > m_skinTransforms[2];
	Ref< render::Buffer > m_jointBuffers[2];
	int32_t m_index;
	mutable Ref< Job > m_updatePoseControllerJob;

	void synchronize() const;

	void updatePoseController(int32_t index, float time, float deltaTime);
};

	}
}

