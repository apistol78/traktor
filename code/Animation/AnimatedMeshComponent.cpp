#include <cmath>
#include "Animation/AnimatedMeshComponent.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/IPoseController.h"
#include "Animation/Joint.h"
#include "Core/Functor/Functor.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/JobManager.h"
#include "Mesh/MeshCulling.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "World/Entity.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

#if !defined(__EMSCRIPTEN__)
//#	define T_USE_UPDATE_JOBS
#endif

namespace traktor
{
	namespace animation
	{
		namespace
		{

render::handle_t s_handleWorld_ShadowWrite;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimatedMeshComponent", AnimatedMeshComponent, mesh::MeshComponent)

AnimatedMeshComponent::AnimatedMeshComponent(
	const Transform& transform,
	const resource::Proxy< mesh::SkinnedMesh >& mesh,
	const resource::Proxy< Skeleton >& skeleton,
	IPoseController* poseController,
	const AlignedVector< int32_t >& jointRemap,
	const AlignedVector< Binding >& bindings,
	bool normalizePose,
	bool normalizeTransform,
	bool screenSpaceCulling
)
:	mesh::MeshComponent(screenSpaceCulling)
,	m_mesh(mesh)
,	m_skeleton(skeleton)
,	m_poseController(poseController)
,	m_jointRemap(jointRemap)
,	m_bindings(bindings)
,	m_normalizePose(normalizePose)
,	m_normalizeTransform(normalizeTransform)
,	m_index(0)
,	m_updateController(1)
,	m_updateTimeScale(1.0f)
{
	s_handleWorld_ShadowWrite = render::getParameterHandle(L"World_ShadowWrite");

	if (m_skeleton)
	{
		calculateJointTransforms(
			m_skeleton,
			m_jointTransforms
		);

		m_poseTransforms.reserve(m_jointTransforms.size());

		size_t skinJointCount = m_mesh->getJointCount();
		m_skinTransforms[0].resize(skinJointCount * 2, Vector4::origo());
		m_skinTransforms[1].resize(skinJointCount * 2, Vector4::origo());
		m_skinTransforms[2].resize(skinJointCount * 2, Vector4::origo());

		updatePoseController(m_index, 0.0f);

		m_index = 1 - m_index;
		m_updateController = 0;
	}
}

AnimatedMeshComponent::~AnimatedMeshComponent()
{
}

void AnimatedMeshComponent::destroy()
{
	synchronize();

	safeDestroy(m_poseController);

	for (auto binding : m_bindings)
		safeDestroy(binding.entity);
	m_bindings.clear();

	mesh::MeshComponent::destroy();
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

void AnimatedMeshComponent::build(world::WorldContext& worldContext, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass)
{
	if (!m_mesh->supportTechnique(worldRenderPass.getTechnique()))
		return;

	synchronize();

	Transform worldTransform = m_transform.get(worldRenderView.getInterval());
	Transform lastWorldTransform = m_transform.get(worldRenderView.getInterval() - 1.0f);

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

	const AlignedVector< Vector4 >& skinTransforms0 = m_skinTransforms[1 - m_index];
	const AlignedVector< Vector4 >& skinTransforms1 = m_skinTransforms[m_index];

	if (skinTransforms0.size() == skinTransforms1.size())
	{
		for (uint32_t i = 0; i < skinTransforms0.size(); ++i)
			m_skinTransforms[2][i] = lerp(
				skinTransforms0[i],
				skinTransforms1[i],
				Scalar(worldRenderView.getInterval())
			);
	}

	m_mesh->build(
		worldContext.getRenderContext(),
		worldRenderPass,
		lastWorldTransform,
		worldTransform,
		m_skinTransforms[2],
		distance,
		getParameterCallback()
	);

	for (auto binding : m_bindings)
		worldContext.build(worldRenderView, worldRenderPass, binding.entity);

	// If only entity's shadow is visible then reduce frequency of controller updates.
	if (m_updateController == 0 && worldRenderPass.getTechnique() == s_handleWorld_ShadowWrite)
	{
		m_updateController = 4;
		m_updateTimeScale = 4.0f;
	}
	else
	{
		m_updateController = 1;
		m_updateTimeScale = 1.0f;
	}
}

void AnimatedMeshComponent::update(const world::UpdateParams& update)
{
	if (m_updateController == 1)
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

		size_t skinJointCount = m_mesh->getJointCount();
		m_skinTransforms[0].resize(skinJointCount * 2, Vector4::origo());
		m_skinTransforms[1].resize(skinJointCount * 2, Vector4::origo());
		m_skinTransforms[2].resize(skinJointCount * 2, Vector4::origo());

		// Prevent further updates from evaluating pose controller,
		// each pose controller needs to set this flag if it's
		// required to continue running even when this entity
		// hasn't been rendered.
		m_updateController = 0;
		m_index = 1 - m_index;

#if defined(T_USE_UPDATE_JOBS)
		m_updatePoseControllerJob = JobManager::getInstance().add(makeFunctor< AnimatedMeshComponent, int32_t, float >(
			this,
			&AnimatedMeshComponent::updatePoseController,
			m_index,
			update.deltaTime
		));
#else
		updatePoseController(m_index, update.deltaTime);
#endif
	}
	else
	{
		m_index = 1 - m_index;
		m_skinTransforms[m_index] = m_skinTransforms[1 - m_index];
	}

	// Update entity to joint bindings.
	for (const auto& binding : m_bindings)
	{
		Transform T;
		if (getPoseTransform(binding.jointHandle, T))
			binding.entity->setTransform(m_transform.get() * T);
		binding.entity->update(update);
	}

	if (m_updateController > 0)
		--m_updateController;

	mesh::MeshComponent::update(update);
}

void AnimatedMeshComponent::setTransform(const Transform& transform)
{
	// Let pose controller know that entity has been manually repositioned.
	if (m_poseController)
		m_poseController->setTransform(transform);

	mesh::MeshComponent::setTransform(transform);
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

		AlignedVector< uint32_t > children;
		m_skeleton->findChildren(index, children);
		for (auto child : children)
			m_poseTransforms[child] = delta * m_jointTransforms[child];
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

void AnimatedMeshComponent::updatePoseController(int32_t index, float deltaTime)
{
	bool updateController = false;

	// Calculate pose transforms and skinning transforms.
	if (m_skeleton && m_poseController)
	{
		m_poseTransforms.resize(0);

		// Evaluate pose transforms in object space.
		m_poseController->evaluate(
			deltaTime * m_updateTimeScale,
			m_transform.get(),
			m_skeleton,
			m_jointTransforms,
			m_poseTransforms,
			updateController
		);

		size_t skeletonJointCount = m_jointTransforms.size();
		size_t skinJointCount = m_mesh->getJointCount();

		// Ensure we have same number of pose transforms as b/ones.
		for (size_t i = m_poseTransforms.size(); i < skeletonJointCount; ++i)
			m_poseTransforms.push_back(m_jointTransforms[i]);

		if (m_normalizePose)
		{
			// Calculate pose offset in object space.
			Vector4 poseOffset = Vector4::zero();
			for (size_t i = 0; i < skeletonJointCount; ++i)
				poseOffset += m_poseTransforms[i].translation();
			poseOffset /= Scalar(float(skeletonJointCount));

			// Normalize pose transforms; update entity transform from offset.
			for (size_t i = 0; i < skeletonJointCount; ++i)
			{
				m_poseTransforms[i] = Transform(
					m_poseTransforms[i].translation() - poseOffset.xyz0(),
					m_poseTransforms[i].rotation()
				);
			}

			if (m_normalizeTransform)
				m_transform.set(Transform(
					m_transform.get().translation() + poseOffset.xyz1(),
					m_transform.get().rotation()
				));
		}

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

	if (updateController)
	{
		m_updateController = 1;
		m_updateTimeScale = 1.0f;
	}
}

	}
}
