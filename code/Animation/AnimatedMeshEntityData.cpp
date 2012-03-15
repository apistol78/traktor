#include "Animation/AnimatedMeshEntity.h"
#include "Animation/AnimatedMeshEntityData.h"
#include "Animation/Bone.h"
#include "Animation/IPoseControllerData.h"
#include "Animation/Skeleton.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Mesh/IMeshResource.h"
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
	if (!resourceManager->bind(m_mesh) || !resourceManager->bind(m_skeleton))
		return 0;

	if (!m_mesh.valid() || !m_skeleton.valid())
		return 0;

	Ref< IPoseController > poseController;
	if (m_poseController)
		poseController = m_poseController->createInstance(
			resourceManager,
			physicsManager,
			m_skeleton,
			getTransform()
		);

	std::vector< int > boneRemap(m_skeleton->getBoneCount());

	const std::map< std::wstring, int >& boneMap = m_mesh->getBoneMap();
	for (int i = 0; i < int(m_skeleton->getBoneCount()); ++i)
	{
		const Bone* bone = m_skeleton->getBone(i);

		std::map< std::wstring, int >::const_iterator j = boneMap.find(bone->getName());
		if (j == boneMap.end())
		{
			log::warning << L"No bone named \"" << bone->getName() << L"\" in skinned mesh" << Endl;
			boneRemap[i] = -1;
			continue;
		}

		boneRemap[i] = j->second;
	}

	return new AnimatedMeshEntity(
		getTransform(),
		m_mesh,
		m_skeleton,
		poseController,
		boneRemap,
		m_normalizePose,
		m_normalizeTransform
	);
}

bool AnimatedMeshEntityData::serialize(ISerializer& s)
{
	if (!world::EntityData::serialize(s))
		return false;
	
	s >> resource::Member< mesh::SkinnedMesh, mesh::IMeshResource >(L"mesh", m_mesh);
	s >> resource::Member< Skeleton >(L"skeleton", m_skeleton);
	s >> MemberRef< IPoseControllerData >(L"poseController", m_poseController);

	if (s.getVersion() >= 1)
	{
		s >> Member< bool >(L"normalizePose", m_normalizePose);
		s >> Member< bool >(L"normalizeTransform", m_normalizeTransform);
	}

	return true;
}

	}
}
