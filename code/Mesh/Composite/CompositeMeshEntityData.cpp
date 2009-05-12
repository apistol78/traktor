#include "Mesh/Composite/CompositeMeshEntityData.h"
#include "Mesh/Composite/CompositeMeshEntity.h"
#include "World/Entity/EntityBuilder.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.mesh.CompositeMeshEntityData", CompositeMeshEntityData, MeshEntityData)

MeshEntity* CompositeMeshEntityData::createEntity(world::EntityBuilder* builder) const
{
	Ref< CompositeMeshEntity > compositeMeshEntity = gc_new< CompositeMeshEntity >(getTransform());
	for (RefArray< MeshEntityData >::const_iterator i = m_meshEntities.begin(); i != m_meshEntities.end(); ++i)
	{
		Ref< MeshEntity > meshEntity = dynamic_type_cast< MeshEntity* >(builder->build(*i));
		if (meshEntity)
			compositeMeshEntity->addMeshEntity((*i)->getName(), meshEntity);
	}
	return compositeMeshEntity;
}

bool CompositeMeshEntityData::serialize(Serializer& s)
{
	if (!MeshEntityData::serialize(s))
		return false;
	return s >> MemberRefArray< MeshEntityData >(L"meshEntities", m_meshEntities);
}

	}
}
