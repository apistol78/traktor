#include "Animation/AnimatedMeshEntity.h"
#include "Animation/AnimatedMeshEntityData.h"
#include "Animation/IPoseControllerData.h"
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.AnimatedMeshEntityData", 1, AnimatedMeshEntityData, world::EntityData)

AnimatedMeshEntityData::AnimatedMeshEntityData()
:	m_normalizePose(false)
,	m_normalizeTransform(false)
{
}

Ref< AnimatedMeshEntity > AnimatedMeshEntityData::createEntity(resource::IResourceManager* resourceManager, physics::PhysicsManager* physicsManager) const
{
	resource::Proxy< mesh::SkinnedMesh > mesh;
	if (!resourceManager->bind(m_mesh, mesh))
		return 0;

	resource::Proxy< Skeleton > skeleton;
	if (!resourceManager->bind(m_skeleton, skeleton))
		return 0;

	Ref< IPoseController > poseController;
	if (m_poseController)
		poseController = m_poseController->createInstance(
			resourceManager,
			physicsManager,
			skeleton,
			getTransform()
		);

	std::vector< int32_t > jointRemap(skeleton->getJointCount());

	const std::map< std::wstring, int32_t >& jointMap = mesh->getJointMap();
	for (uint32_t i = 0; i < skeleton->getJointCount(); ++i)
	{
		const Joint* joint = skeleton->getJoint(i);

		std::map< std::wstring, int32_t >::const_iterator j = jointMap.find(joint->getName());
		if (j == jointMap.end())
		{
			jointRemap[i] = -1;
			continue;
		}

		jointRemap[i] = j->second;
	}

	return new AnimatedMeshEntity(
		getTransform(),
		mesh,
		skeleton,
		poseController,
		jointRemap,
		m_normalizePose,
		m_normalizeTransform
	);
}

void AnimatedMeshEntityData::serialize(ISerializer& s)
{
	world::EntityData::serialize(s);
	
	s >> resource::Member< mesh::SkinnedMesh >(L"mesh", m_mesh);
	s >> resource::Member< Skeleton >(L"skeleton", m_skeleton);
	s >> MemberRef< IPoseControllerData >(L"poseController", m_poseController);

	if (s.getVersion() >= 1)
	{
		s >> Member< bool >(L"normalizePose", m_normalizePose);
		s >> Member< bool >(L"normalizeTransform", m_normalizeTransform);
	}
}

	}
}
