#include "Mesh/Composite/CompositeMeshEntityData.h"
#include "Mesh/Composite/CompositeMeshEntity.h"
#include "World/Entity/IEntityBuilder.h"
#include "World/Entity/EntityInstance.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.mesh.CompositeMeshEntityData", CompositeMeshEntityData, MeshEntityData)

Ref< MeshEntity > CompositeMeshEntityData::createEntity(resource::IResourceManager* resourceManager, world::IEntityBuilder* builder) const
{
	Ref< CompositeMeshEntity > compositeMeshEntity = gc_new< CompositeMeshEntity >(cref(getTransform()));
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

bool CompositeMeshEntityData::serialize(Serializer& s)
{
	if (!MeshEntityData::serialize(s))
		return false;
	return s >> MemberRefArray< world::EntityInstance >(L"instances", m_instances);
}

	}
}
