/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cmath>
#include "Animation/AnimatedMeshComponent.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/IPoseController.h"
#include "Animation/Joint.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/JobManager.h"
#include "Mesh/MeshCulling.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Render/Buffer.h"
#include "World/Entity.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

#define T_USE_UPDATE_JOBS

namespace traktor
{
	namespace animation
	{
		namespace
		{

const render::Handle s_techniqueVelocityWrite(L"World_VelocityWrite");
const render::Handle s_handleWorld_ShadowWrite(L"World_ShadowWrite");

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimatedMeshComponent", AnimatedMeshComponent, mesh::MeshComponent)

AnimatedMeshComponent::AnimatedMeshComponent(
	const Transform& transform,
	const resource::Proxy< mesh::SkinnedMesh >& mesh,
	const resource::Proxy< Skeleton >& skeleton,
	IPoseController* poseController,
	const AlignedVector< int32_t >& jointRemap,
	const AlignedVector< Binding >& bindings,
	render::IRenderSystem* renderSystem,
	bool screenSpaceCulling
)
:	mesh::MeshComponent(screenSpaceCulling)
,	m_mesh(mesh)
,	m_skeleton(skeleton)
,	m_poseController(poseController)
,	m_jointRemap(jointRemap)
,	m_bindings(bindings)
,	m_index(0)
{
	const uint32_t skinJointCount = m_mesh->getJointCount();

	m_jointBuffers[0] = mesh::SkinnedMesh::createJointBuffer(renderSystem, skinJointCount);
	m_jointBuffers[1] = mesh::SkinnedMesh::createJointBuffer(renderSystem, skinJointCount);

	if (m_skeleton)
	{
		calculateJointTransforms(
			m_skeleton,
			m_jointTransforms
		);

		m_poseTransforms.reserve(m_jointTransforms.size());

		m_skinTransforms[0].resize(skinJointCount * 2, Vector4::origo());
		m_skinTransforms[1].resize(skinJointCount * 2, Vector4::origo());

		updatePoseController(m_index, 0.0f, 0.0f);
		m_index = 1 - m_index;
	}
}

AnimatedMeshComponent::~AnimatedMeshComponent()
{
}

void AnimatedMeshComponent::destroy()
{
	synchronize();

	safeDestroy(m_jointBuffers[1]);
	safeDestroy(m_jointBuffers[0]);
	safeDestroy(m_poseController);

	for (auto binding : m_bindings)
		safeDestroy(binding.entity);
	m_bindings.clear();

	mesh::MeshComponent::destroy();
}

void AnimatedMeshComponent::setOwner(world::Entity* owner)
{
	mesh::MeshComponent::setOwner(owner);
}

void AnimatedMeshComponent::setTransform(const Transform& transform)
{
	// Let pose controller know that entity has been manually repositioned.
	if (m_poseController)
		m_poseController->setTransform(transform);

	mesh::MeshComponent::setTransform(transform);
}

Aabb3 AnimatedMeshComponent::getBoundingBox() const
{
	synchronize();

	Aabb3 boundingBox = m_mesh->getBoundingBox();

	if (!m_poseTransforms.empty())
	{
		for (uint32_t i = 0; i < uint32_t(m_poseTransforms.size()); ++i)
		{
			const Joint* joint = m_skeleton->getJoint(i);
			float radius = joint->getRadius();
			boundingBox.contain(m_poseTransforms[i].translation().xyz1(), Scalar(radius));
		}
	}

	return boundingBox;
}

void AnimatedMeshComponent::update(const world::UpdateParams& update)
{
	synchronize();

	// Calculate original bone transforms in object space.
	if (m_skeleton.changed())
	{
		m_jointTransforms.resize(0);
		m_poseTransforms.resize(0);

		if (m_skeleton)
			calculateJointTransforms(
				m_skeleton,
				m_jointTransforms
			);

		m_poseTransforms.reserve(m_jointTransforms.size());
		m_skeleton.consume();
	}

	// Always ensure skin arrays are same size as mesh joints.
	size_t skinJointCount = m_mesh->getJointCount();
	m_skinTransforms[0].resize(skinJointCount * 2, Vector4::origo());
	m_skinTransforms[1].resize(skinJointCount * 2, Vector4::origo());
	m_index = 1 - m_index;

#if defined(T_USE_UPDATE_JOBS)
	m_updatePoseControllerJob = JobManager::getInstance().add([=](){
		updatePoseController(m_index, update.alternateTime, update.deltaTime);
	});
#else
	updatePoseController(m_index, update.alternateTime, update.deltaTime);
#endif

	// Update entity to joint bindings.
	for (const auto& binding : m_bindings)
	{
		Transform T;
		if (getPoseTransform(binding.jointHandle, T))
			binding.entity->setTransform(m_transform.get() * T);
		binding.entity->update(update);
	}

	mesh::MeshComponent::update(update);
}

void AnimatedMeshComponent::build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass)
{
	if (!m_mesh->supportTechnique(worldRenderPass.getTechnique()))
		return;

	synchronize();

	const Scalar interval(worldRenderView.getInterval());

	Transform worldTransform = m_transform.get(interval);
	Transform lastWorldTransform = m_transform.get(interval - 1.0_simd);

	float distance = 0.0f;
	if (!mesh::isMeshVisible(
		m_mesh->getBoundingBox(),
		worldRenderView.getCullFrustum(),
		worldRenderView.getView() * worldTransform.toMatrix44(),
		worldRenderView.getProjection(),
		m_screenSpaceCulling ? 0.0001f : 0.0f,
		distance
	))
		return;

	auto jointBufferLast = m_jointBuffers[0];
	auto jointBufferCurrent = m_jointBuffers[1];

	// Update joint buffers only for first pass of frame, buffers are implicitly double buffered
	// and cannot be updated multiple times per frame.
	if ((worldRenderPass.getPassFlags() & world::IWorldRenderPass::PfFirst) != 0)
	{
		const auto& skinTransformsLastUpdate = m_skinTransforms[1 - m_index];
		const auto& skinTransformsCurrentUpdate = m_skinTransforms[m_index];

		// Interpolate between updates to get current build skin transforms.
		mesh::SkinnedMesh::JointData* jointData = (mesh::SkinnedMesh::JointData*)jointBufferCurrent->lock();
		for (uint32_t i = 0; i < skinTransformsCurrentUpdate.size(); i += 2)
		{
			auto translation = lerp(skinTransformsLastUpdate[i + 1], skinTransformsCurrentUpdate[i + 1], interval);
			auto rotation = lerp(skinTransformsLastUpdate[i], skinTransformsCurrentUpdate[i], interval);
			translation.storeAligned(jointData->translation);
			rotation.storeAligned(jointData->rotation);
			jointData++;
		}
		jointBufferCurrent->unlock();
	}

	m_mesh->build(
		context.getRenderContext(),
		worldRenderPass,
		lastWorldTransform,
		worldTransform,
		jointBufferLast,
		jointBufferCurrent,
		distance,
		getParameterCallback()
	);

	// Save last rendered transform so we can properly write velocities next frame.
	if ((worldRenderPass.getPassFlags() & world::IWorldRenderPass::PfLast) != 0)
	{
		m_jointBuffers[0] = jointBufferCurrent;
		m_jointBuffers[1] = jointBufferLast;
	}
}

bool AnimatedMeshComponent::getJointTransform(render::handle_t jointName, Transform& outTransform) const
{
	uint32_t index;
	if (!m_skeleton->findJoint(jointName, index))
		return false;

	synchronize();

	if (index >= m_jointTransforms.size())
		return false;

	outTransform = m_jointTransforms[index];
	return true;
}

bool AnimatedMeshComponent::getPoseTransform(render::handle_t jointName, Transform& outTransform) const
{
	uint32_t index;
	if (!m_skeleton->findJoint(jointName, index))
		return false;

	synchronize();

	if (index >= m_poseTransforms.size())
		return false;

	outTransform = m_poseTransforms[index];
	return true;
}

bool AnimatedMeshComponent::getSkinTransform(render::handle_t jointName, Transform& outTransform) const
{
	uint32_t index;
	if (!m_skeleton->findJoint(jointName, index))
		return false;

	synchronize();

	if (index >= m_poseTransforms.size())
		return false;

	int skinIndex = m_jointRemap[index];
	if (skinIndex < 0)
		return false;

	Quaternion tmp;
	m_skinTransforms[m_index][skinIndex * 2 + 0].storeAligned((float*)&tmp);

	outTransform = Transform(
		m_skinTransforms[m_index][skinIndex * 2 + 1],
		tmp
	);
	return true;
}

bool AnimatedMeshComponent::setPoseTransform(render::handle_t jointName, const Transform& transform, bool inclusive)
{
	uint32_t index;
	if (!m_skeleton->findJoint(jointName, index))
		return false;

	synchronize();

	if (index >= m_jointTransforms.size())
		return false;

	if (m_poseTransforms.empty())
		m_poseTransforms = m_jointTransforms;

	m_poseTransforms[index] = transform;

	if (inclusive)
	{
		Transform delta = transform * m_jointTransforms[index].inverse();
		m_skeleton->findAllChildren(index, [&](uint32_t child){
			m_poseTransforms[child] = delta * m_jointTransforms[child];
		});
	}

	return true;
}

void AnimatedMeshComponent::synchronize() const
{
#if defined(T_USE_UPDATE_JOBS)
	if (m_updatePoseControllerJob)
	{
		m_updatePoseControllerJob->wait();
		m_updatePoseControllerJob = nullptr;
	}
#endif
}

void AnimatedMeshComponent::updatePoseController(int32_t index, float time, float deltaTime)
{
	// Calculate pose transforms and skinning transforms.
	if (m_skeleton && m_poseController)
	{
		m_poseTransforms.resize(0);

		// Evaluate pose transforms in object space.
		m_poseController->evaluate(
			time,
			deltaTime,
			m_transform.get(),
			m_skeleton,
			m_jointTransforms,
			m_poseTransforms
		);

		size_t skeletonJointCount = m_jointTransforms.size();
		size_t skinJointCount = m_mesh->getJointCount();

		// Ensure we have same number of pose transforms as bones.
		for (size_t i = m_poseTransforms.size(); i < skeletonJointCount; ++i)
			m_poseTransforms.push_back(m_jointTransforms[i]);

		// Calculate skin transforms in delta space.
		for (size_t i = 0; i < skeletonJointCount; ++i)
		{
			int32_t jointIndex = m_jointRemap[i];
			if (jointIndex >= 0 && jointIndex < int32_t(skinJointCount))
			{
				Transform skinTransform = m_poseTransforms[i] * m_jointTransforms[i].inverse();
				m_skinTransforms[index][jointIndex * 2 + 0] = skinTransform.rotation().e;
				m_skinTransforms[index][jointIndex * 2 + 1] = skinTransform.translation().xyz1();
			}
		}
	}
	else if (!m_poseTransforms.empty())
	{
		size_t skeletonJointCount = m_jointTransforms.size();
		size_t skinJointCount = m_mesh->getJointCount();

		// Calculate skin transforms in delta space.
		for (size_t i = 0; i < skeletonJointCount; ++i)
		{
			int32_t jointIndex = m_jointRemap[i];
			if (jointIndex >= 0 && jointIndex < int32_t(skinJointCount))
			{
				Transform skinTransform = m_poseTransforms[i] * m_jointTransforms[i].inverse();
				m_skinTransforms[index][jointIndex * 2 + 0] = skinTransform.rotation().e;
				m_skinTransforms[index][jointIndex * 2 + 1] = skinTransform.translation().xyz1();
			}
		}
	}
}

	}
}
