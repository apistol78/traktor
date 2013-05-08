#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Mesh/Composite/CompositeMeshEntity.h"
#include "Mesh/Composite/CompositeMeshEntityData.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.mesh.CompositeMeshEntityData", 0, CompositeMeshEntityData, AbstractMeshEntityData)

Ref< MeshEntity > CompositeMeshEntityData::createEntity(resource::IResourceManager* resourceManager, const world::IEntityBuilder* builder) const
{
	Ref< CompositeMeshEntity > compositeMeshEntity = new CompositeMeshEntity(getTransform());
	for (RefArray< AbstractMeshEntityData >::const_iterator i = m_entityData.begin(); i != m_entityData.end(); ++i)
	{
		Ref< MeshEntity > meshEntity = dynamic_type_cast< MeshEntity* >(builder->create(*i));
		if (meshEntity)
			compositeMeshEntity->addMeshEntity((*i)->getName(), meshEntity);
		else
			log::error << L"Unable to create mesh entity from instance \"" << (*i)->getName() << L"\"" << Endl;
	}
	return compositeMeshEntity;
}

void CompositeMeshEntityData::setTransform(const Transform& transform)
{
	Transform deltaTransform = getTransform().inverse() * transform;
	for (RefArray< AbstractMeshEntityData >::iterator i = m_entityData.begin(); i != m_entityData.end(); ++i)
	{
		Transform currentTransform = (*i)->getTransform();
		(*i)->setTransform(currentTransform * deltaTransform);
	}
	AbstractMeshEntityData::setTransform(transform);
}

void CompositeMeshEntityData::serialize(ISerializer& s)
{
	AbstractMeshEntityData::serialize(s);
	s >> MemberRefArray< AbstractMeshEntityData >(L"entityData", m_entityData);
}

void CompositeMeshEntityData::addEntityData(AbstractMeshEntityData* entityData)
{
	m_entityData.push_back(entityData);
}

void CompositeMeshEntityData::removeEntityData(AbstractMeshEntityData* entityData)
{
	m_entityData.remove(entityData);
}

	}
}
