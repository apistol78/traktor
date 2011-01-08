#include <cmath>
#include "Animation/AnimatedMeshEntity.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/Bone.h"
#include "Animation/IPoseController.h"
#include "Core/Misc/SafeDestroy.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"
#include "World/Entity/EntityUpdate.h"

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

Aabb AnimatedMeshEntity::getBoundingBox() const
{
	synchronize();

	Aabb boundingBox = m_mesh.valid() ? m_mesh->getBoundingBox() : Aabb();

	if (!m_poseTransforms.empty())
	{
		for (uint32_t i = 0; i < uint32_t(m_poseTransforms.size()); ++i)
		{
			const Bone* bone = m_skeleton->getBone(i);

			float length = bone->getLength();
			float radius = bone->getRadius();

			Aabb boneLocalAabb(Vector4(-radius, -radius, 0.0f), Vector4(radius, radius, length));
			Aabb boneAabb = boneLocalAabb.transform(m_poseTransforms[i]);

			boundingBox.contain(boneAabb);
		}
	}

	return boundingBox;
}

void AnimatedMeshEntity::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	float distance
)
{
	if (!m_mesh.validate())
		return;

	synchronize();

	m_mesh->render(
		worldContext.getRenderContext(),
		worldRenderPass,
		m_transform,
		m_skinTransforms,
		distance,
		getParameterCallback()
	);

	m_first = false;
	m_updateController = true;
}

void AnimatedMeshEntity::update(const world::EntityUpdate* update)
{
	if (!m_skeleton.validate() || !m_updateController)
		return;

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
		update->getDeltaTime()
	));
#else
	updatePoseController(update->getDeltaTime());
#endif
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

	if (!m_skeleton.validate() || !m_skeleton->findBone(boneName, index))
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

	if (!m_skeleton.validate() || !m_skeleton->findBone(boneName, index))
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

	if (!m_skeleton.validate() || !m_skeleton->findBone(boneName, index))
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
	if (m_poseController)
	{
		m_boneTransforms.resize(0);
		m_poseTransforms.resize(0);

		// Calculate original bone transforms in object space.
		calculateBoneTransforms(
			m_skeleton,
			m_boneTransforms
		);

		// Evaluate pose transforms in object space.
		m_poseController->evaluate(
			deltaTime,
			m_transform,
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
				m_transform = Transform(
					m_transform.translation() + poseOffset.xyz1(),
					m_transform.rotation()
				);
		}

		// Initialize skin transforms.
		m_skinTransforms.resize(skinBoneCount * 2);
		for (size_t i = 0; i < skinBoneCount * 2; i += 2)
		{
			m_skinTransforms[i + 0] = Vector4::origo();
			m_skinTransforms[i + 1] = Vector4::origo();
		}

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
		m_skinTransforms.resize(skinBoneCount * 2);
		for (size_t i = 0; i < skinBoneCount * 2; i += 2)
		{
			m_skinTransforms[i + 0] = Vector4::origo();
			m_skinTransforms[i + 1] = Vector4::origo();
		}
	}
}

	}
}
