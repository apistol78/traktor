#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Mesh/Composite/CompositeMeshEntity.h"
#include "Mesh/Composite/CompositeMeshEntityData.h"
#include "World/Entity/EntityInstance.h"
#include "World/Entity/IEntityBuilder.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.mesh.CompositeMeshEntityData", 0, CompositeMeshEntityData, AbstractMeshEntityData)

Ref< MeshEntity > CompositeMeshEntityData::createEntity(resource::IResourceManager* resourceManager, world::IEntityBuilder* builder) const
{
	Ref< CompositeMeshEntity > compositeMeshEntity = new CompositeMeshEntity(getTransform());
	for (RefArray< world::EntityInstance >::const_iterator i = m_instances.begin(); i != m_instances.end(); ++i)
	{
		T_FATAL_ASSERT(*i);
		Ref< MeshEntity > meshEntity = dynamic_type_cast< MeshEntity* >(builder->build(*i));
		if (meshEntity)
			compositeMeshEntity->addMeshEntity((*i)->getName(), meshEntity);
		else
			log::error << L"Unable to create mesh entity from instance \"" << (*i)->getName() << L"\"" << Endl;
	}
	return compositeMeshEntity;
}

bool CompositeMeshEntityData::serialize(ISerializer& s)
{
	if (!AbstractMeshEntityData::serialize(s))
		return false;
	return s >> MemberRefArray< world::EntityInstance >(L"instances", m_instances);
}

	}
}
