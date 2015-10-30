#include "Mesh/AbstractMeshEntityData.h"
#include "Mesh/MeshComponent.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/MeshEntity.h"
#include "Mesh/MeshEntityFactory.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshEntityFactory", MeshEntityFactory, world::IEntityFactory)

MeshEntityFactory::MeshEntityFactory(resource::IResourceManager* resourceManager)
:	m_resourceManager(resourceManager)
{
}

const TypeInfoSet MeshEntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< AbstractMeshEntityData >());
	return typeSet;
}

const TypeInfoSet MeshEntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet MeshEntityFactory::getEntityComponentTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< MeshComponentData >());
	return typeSet;
}

Ref< world::Entity > MeshEntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	const AbstractMeshEntityData* meshEntityData = mandatory_non_null_type_cast< const AbstractMeshEntityData* >(&entityData);
	return meshEntityData->createEntity(m_resourceManager, builder);
}

Ref< world::IEntityEvent > MeshEntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return 0;
}

Ref< world::IEntityComponent > MeshEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, world::Entity* owner, const world::IEntityComponentData& entityComponentData) const
{
	const MeshComponentData* meshComponentData = mandatory_non_null_type_cast< const MeshComponentData* >(&entityComponentData);
	return meshComponentData->createComponent(owner, m_resourceManager);
}

	}
}
