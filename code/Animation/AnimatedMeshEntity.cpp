#include <cmath>
#include "Animation/AnimatedMeshEntity.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/Bone.h"
#include "Animation/IPoseController.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"
#include "World/Entity/EntityUpdate.h"

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
	const std::vector< int >& boneRemap
)
:	mesh::MeshEntity(transform)
,	m_mesh(mesh)
,	m_skeleton(skeleton)
,	m_poseController(poseController)
,	m_boneRemap(boneRemap)
,	m_totalTime(0.0f)
,	m_updateController(true)
{
}

Aabb AnimatedMeshEntity::getBoundingBox() const
{
	if (!m_poseTransforms.empty())
	{
		Aabb boundingBox;
		for (uint32_t i = 0; i < uint32_t(m_poseTransforms.size()); ++i)
		{
			const Bone* bone = m_skeleton->getBone(i);

			float length = bone->getLength();
			float radius = bone->getRadius();

			Aabb boneLocalAabb(Vector4(-radius, -radius, 0.0f), Vector4(radius, radius, length));
			Aabb boneAabb = boneLocalAabb.transform(m_poseTransforms[i]);

			boundingBox.contain(boneAabb);
		}
		return boundingBox;
	}
	return m_mesh.valid() ? m_mesh->getBoundingBox() : Aabb();
}

void AnimatedMeshEntity::render(world::WorldContext* worldContext, world::WorldRenderView* worldRenderView, float distance)
{
	if (!m_mesh.validate())
		return;

	m_mesh->render(
		worldContext->getRenderContext(),
		worldRenderView,
		m_transform,
		m_skinTransforms,
		distance,
		getParameterCallback()
	);

	m_updateController = true;
}

void AnimatedMeshEntity::update(const world::EntityUpdate* update)
{
	if (!m_updateController)
		return;

	// Prevent further updates from evaluating pose controller,
	// each pose controller needs to set this flag if it's
	// required to continue running even when this entity
	// hasn't been rendered.
	m_updateController = false;

	if (m_poseController)
	{
		m_boneTransforms.resize(0);
		m_poseTransforms.resize(0);

		calculateBoneTransforms(
			m_skeleton,
			m_boneTransforms
		);

		m_poseController->evaluate(
			update->getDeltaTime(),
			m_transform,
			m_skeleton,
			m_boneTransforms,
			m_poseTransforms,
			m_updateController
		);

		while (m_poseTransforms.size() < m_boneTransforms.size())
			m_poseTransforms.push_back(Transform::identity());

		m_skinTransforms.resize(m_boneTransforms.size());
		
		for (size_t i = 0; i < m_boneTransforms.size(); ++i)
			m_skinTransforms[i] = Matrix44::identity();

		for (size_t i = 0; i < m_boneTransforms.size(); ++i)
		{
			int boneIndex = m_boneRemap[i];
			if (boneIndex >= 0)
				m_skinTransforms[boneIndex] = (m_poseTransforms[i] * m_boneTransforms[i].inverse()).toMatrix44();
		}
	}
	else
	{
		m_skinTransforms.resize(m_skeleton->getBoneCount());
		for (size_t i = 0; i < m_skinTransforms.size(); ++i)
			m_skinTransforms[i] = Matrix44::identity();
	}
}

bool AnimatedMeshEntity::getBoneTransform(const std::wstring& boneName, Transform& outTransform) const
{
	uint32_t index;

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

	if (!m_skeleton.validate() || !m_skeleton->findBone(boneName, index))
		return false;
	if (index >= m_poseTransforms.size())
		return false;

	outTransform = m_poseTransforms[index];
	return true;
}

bool AnimatedMeshEntity::getSkinTransform(const std::wstring& boneName, Matrix44& outTransform) const
{
	uint32_t index;

	if (!m_skeleton.validate() || !m_skeleton->findBone(boneName, index))
		return false;
	if (index >= m_poseTransforms.size())
		return false;

	int skinIndex = m_boneRemap[index];
	if (skinIndex < 0)
		return false;

	outTransform = m_skinTransforms[skinIndex];
	return true;
}

	}
}
