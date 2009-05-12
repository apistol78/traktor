#include "Animation/AnimatedMeshEntityData.h"
#include "Animation/AnimatedMeshEntity.h"
#include "Animation/PoseControllerData.h"
#include "Animation/Skeleton.h"
#include "Animation/Bone.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Skinned/SkinnedMeshResource.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/Member.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.animation.AnimatedMeshEntityData", AnimatedMeshEntityData, world::SpatialEntityData)

AnimatedMeshEntity* AnimatedMeshEntityData::createEntity(physics::PhysicsManager* physicsManager) const
{
	resource::Proxy< mesh::SkinnedMesh > mesh = m_mesh;
	if (!mesh.validate())
		return 0;

	resource::Proxy< Skeleton > skeleton = m_skeleton;
	if (!skeleton.validate())
		return 0;

	Ref< PoseController > poseController;
	if (m_poseController)
		poseController = m_poseController->createInstance(
			physicsManager,
			skeleton,
			getTransform()
		);

	std::vector< int > boneRemap(skeleton->getBoneCount());

	const std::map< std::wstring, int >& boneMap = mesh->getBoneMap();
	for (int i = 0; i < int(skeleton->getBoneCount()); ++i)
	{
		const Bone* bone = skeleton->getBone(i);

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
		cref(mesh),
		cref(skeleton),
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
	s >> MemberRef< PoseControllerData >(L"poseController", m_poseController);

	return true;
}

	}
}
