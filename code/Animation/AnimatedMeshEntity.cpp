#include <cmath>
#include "Animation/AnimatedMeshEntity.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/Bone.h"
#include "Animation/IPoseController.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/JobManager.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

//#define T_USE_UPDATE_JOBS

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimatedMeshEntity", AnimatedMeshEntity, mesh::MeshEntity)

AnimatedMeshEntity::AnimatedMeshEntity(
	const Transform& transform,
	const resource::Proxy< mesh::SkinnedMesh >& mesh,
	const resource::Proxy< Skeleton >& skeleton,
	IPoseController* poseController,
	const std::vector< int >& boneRemap,
	bool normalizePose,
	bool normalizeTransform
)
:	mesh::MeshEntity(transform)
,	m_mesh(mesh)
,	m_skeleton(skeleton)
,	m_poseController(poseController)
,	m_boneRemap(boneRemap)
,	m_normalizePose(normalizePose)
,	m_normalizeTransform(normalizeTransform)
,	m_totalTime(0.0f)
,	m_updateController(true)
{
}

AnimatedMeshEntity::~AnimatedMeshEntity()
{
	destroy();
}

void AnimatedMeshEntity::destroy()
{
	synchronize();
	safeDestroy(m_poseController);
	mesh::MeshEntity::destroy();
}

Aabb3 AnimatedMeshEntity::getBoundingBox() const
{
	synchronize();

	Aabb3 boundingBox = m_mesh->getBoundingBox();

	if (!m_poseTransforms.empty())
	{
		for (uint32_t i = 0; i < uint32_t(m_poseTransforms.size()); ++i)
		{
			const Bone* bone = m_skeleton->getBone(i);

			float length = bone->getLength();
			float radius = bone->getRadius();

			Aabb3 boneLocalAabb(Vector4(-radius, -radius, 0.0f), Vector4(radius, radius, length));
			Aabb3 boneAabb = boneLocalAabb.transform(m_poseTransforms[i]);

			boundingBox.contain(boneAabb);
		}
	}

	return boundingBox;
}

bool AnimatedMeshEntity::supportTechnique(render::handle_t technique) const
{
	return m_mesh->supportTechnique(technique);
}

void AnimatedMeshEntity::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	float distance
)
{
	synchronize();

	m_mesh->render(
		worldContext.getRenderContext(),
		worldRenderPass,
		getTransform(worldRenderView.getInterval()),
		m_skinTransforms,
		distance,
		getParameterCallback()
	);

	m_updateController = true;
}

void AnimatedMeshEntity::update(const UpdateParams& update)
{
	if (m_updateController)
	{
		synchronize();

		// Prevent further updates from evaluating pose controller,
		// each pose controller needs to set this flag if it's
		// required to continue running even when this entity
		// hasn't been rendered.
		m_updateController = false;

#if defined(T_USE_UPDATE_JOBS)
		m_updatePoseControllerJob = JobManager::getInstance().add(makeFunctor< AnimatedMeshEntity, float >(
			this,
			&AnimatedMeshEntity::updatePoseController,
			update.deltaTime
		));
#else
		updatePoseController(update.deltaTime);
#endif
	}

	mesh::MeshEntity::update(update);
}

void AnimatedMeshEntity::setTransform(const Transform& transform)
{
	// Let pose controller know that entity has been manually repositioned.
	if (m_poseController)
		m_poseController->setTransform(transform);

	mesh::MeshEntity::setTransform(transform);
}

bool AnimatedMeshEntity::getBoneTransform(const std::wstring& boneName, Transform& outTransform) const
{
	uint32_t index;

	synchronize();

	if (!m_skeleton->findBone(boneName, index))
		return false;
	if (index >= m_boneTransforms.size())
		return false;

	outTransform = m_boneTransforms[index];
	return true;
}

bool AnimatedMeshEntity::getPoseTransform(const std::wstring& boneName, Transform& outTransform) const
{
	uint32_t index;

	synchronize();

	if (!m_skeleton->findBone(boneName, index))
		return false;
	if (index >= m_poseTransforms.size())
		return false;

	outTransform = m_poseTransforms[index];
	return true;
}

bool AnimatedMeshEntity::getSkinTransform(const std::wstring& boneName, Transform& outTransform) const
{
	uint32_t index;

	synchronize();

	if (!m_skeleton->findBone(boneName, index))
		return false;
	if (index >= m_poseTransforms.size())
		return false;

	int skinIndex = m_boneRemap[index];
	if (skinIndex < 0)
		return false;

	Quaternion tmp;
	m_skinTransforms[skinIndex * 2 + 0].storeAligned((float*)&tmp);
	
	outTransform = Transform(
		m_skinTransforms[skinIndex * 2 + 1],
		tmp
	);
	return true;
}

bool AnimatedMeshEntity::setPoseTransform(const std::wstring& boneName, const Transform& transform, bool inclusive)
{
	uint32_t index;

	synchronize();

	if (!m_skeleton->findBone(boneName, index))
		return false;

	if (index >= m_boneTransforms.size())
		return false;

	if (m_poseTransforms.empty())
		m_poseTransforms = m_boneTransforms;

	m_poseTransforms[index] = transform;

	if (inclusive)
	{
		Transform delta = transform * m_boneTransforms[index].inverse();

		std::vector< uint32_t > children;
		m_skeleton->findChildren(index, children);

		for (std::vector< uint32_t >::const_iterator i = children.begin(); i != children.end(); ++i)
			m_poseTransforms[*i] = m_boneTransforms[*i] * delta;
	}

	return true;
}

void AnimatedMeshEntity::synchronize() const
{
#if defined(T_USE_UPDATE_JOBS)
	if (m_updatePoseControllerJob)
	{
		m_updatePoseControllerJob->wait();
		m_updatePoseControllerJob = 0;
	}
#endif
}

void AnimatedMeshEntity::updatePoseController(float deltaTime)
{
	// Calculate original bone transforms in object space.
	if (m_skeleton.changed())
	{
		m_boneTransforms.resize(0);
		m_poseTransforms.resize(0);

		calculateBoneTransforms(
			m_skeleton,
			m_boneTransforms
		);

		m_skeleton.consume();
	}

	// Calculate pose transforms and skinning transforms.
	if (m_poseController)
	{
		m_poseTransforms.resize(0);

		// Evaluate pose transforms in object space.
		m_poseController->evaluate(
			deltaTime,
			m_transform.get(),
			m_skeleton,
			m_boneTransforms,
			m_poseTransforms,
			m_updateController
		);

		size_t skeletonBoneCount = m_boneTransforms.size();
		size_t skinBoneCount = m_mesh->getBoneCount();
		
		// Ensure we have same number of pose transforms as bones.
		for (size_t i = m_poseTransforms.size(); i < skeletonBoneCount; ++i)
			m_poseTransforms.push_back(m_boneTransforms[i]);

		if (m_normalizePose)
		{
			// Calculate pose offset in object space.
			Vector4 poseOffset = Vector4::zero();
			for (size_t i = 0; i < skeletonBoneCount; ++i)
				poseOffset += m_poseTransforms[i].translation();
			poseOffset /= Scalar(float(skeletonBoneCount));
	
			// Normalize pose transforms; update entity transform from offset.
			for (size_t i = 0; i < skeletonBoneCount; ++i)
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

		// Initialize skin transforms.
		m_skinTransforms.resize(skinBoneCount * 2, Vector4::origo());

		// Calculate skin transforms in delta space.
		for (size_t i = 0; i < skeletonBoneCount; ++i)
		{
			int32_t boneIndex = m_boneRemap[i];
			if (boneIndex >= 0 && boneIndex < int32_t(skinBoneCount))
			{
				Transform skinTransform = m_poseTransforms[i] * m_boneTransforms[i].inverse();
				m_skinTransforms[boneIndex * 2 + 0] = skinTransform.rotation().e;
				m_skinTransforms[boneIndex * 2 + 1] = skinTransform.translation().xyz1();
			}
		}
	}
	else if (!m_poseTransforms.empty())
	{
		size_t skeletonBoneCount = m_boneTransforms.size();
		size_t skinBoneCount = m_mesh->getBoneCount();

		// Initialize skin transforms.
		m_skinTransforms.resize(skinBoneCount * 2, Vector4::origo());

		// Calculate skin transforms in delta space.
		for (size_t i = 0; i < skeletonBoneCount; ++i)
		{
			int32_t boneIndex = m_boneRemap[i];
			if (boneIndex >= 0 && boneIndex < int32_t(skinBoneCount))
			{
				Transform skinTransform = m_poseTransforms[i] * m_boneTransforms[i].inverse();
				m_skinTransforms[boneIndex * 2 + 0] = skinTransform.rotation().e;
				m_skinTransforms[boneIndex * 2 + 1] = skinTransform.translation().xyz1();
			}
		}
	}
	else
	{
		size_t skinBoneCount = m_mesh->getBoneCount();
		m_skinTransforms.resize(skinBoneCount * 2, Vector4::origo());
	}
}

	}
}
