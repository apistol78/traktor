#include "Animation/AnimatedMeshEntityData.h"
#include "Animation/AnimatedMeshEntity.h"
#include "Animation/IPoseControllerData.h"
#include "Animation/Skeleton.h"
#include "Animation/Bone.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Skinned/SkinnedMeshResource.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.animation.AnimatedMeshEntityData", AnimatedMeshEntityData, world::SpatialEntityData)

AnimatedMeshEntity* AnimatedMeshEntityData::createEntity(resource::IResourceManager* resourceManager, physics::PhysicsManager* physicsManager) const
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

	return gc_new< AnimatedMeshEntity >(
		cref(getTransform()),
		cref(m_mesh),
		cref(m_skeleton),
		poseController,
		cref(boneRemap)
	);
}

bool AnimatedMeshEntityData::serialize(Serializer& s)
{
	if (!world::SpatialEntityData::serialize(s))
		return false;
	
	s >> resource::Member< mesh::SkinnedMesh, mesh::SkinnedMeshResource >(L"mesh", m_mesh);
	s >> resource::Member< Skeleton >(L"skeleton", m_skeleton);
	s >> MemberRef< IPoseControllerData >(L"poseController", m_poseController);

	return true;
}

	}
}
