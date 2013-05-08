#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Mesh/Editor/Batch/BatchMeshEntityData.h"
#include "World/EntityData.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.mesh.BatchMeshEntityData", 0, BatchMeshEntityData, world::EntityData)

BatchMeshEntityData::BatchMeshEntityData()
:	m_outputGuid(Guid::create())
{
}

void BatchMeshEntityData::setTransform(const Transform& transform)
{
	Transform deltaTransform = getTransform().inverse() * transform;
	for (RefArray< world::EntityData >::iterator i = m_entityData.begin(); i != m_entityData.end(); ++i)
	{
		Transform currentTransform = (*i)->getTransform();
		(*i)->setTransform(currentTransform * deltaTransform);
	}
	world::EntityData::setTransform(transform);
}

void BatchMeshEntityData::serialize(ISerializer& s)
{
	world::EntityData::serialize(s);
	
	s >> Member< Guid >(L"outputGuid", m_outputGuid);
	s >> MemberRefArray< world::EntityData >(L"entityData", m_entityData);
}

	}
}
