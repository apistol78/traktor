#include "Mesh/MeshComponent.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/MeshEntityFactory.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshEntityFactory", MeshEntityFactory, world::IEntityFactory)

MeshEntityFactory::MeshEntityFactory(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
{
}

const TypeInfoSet MeshEntityFactory::getEntityTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet MeshEntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet MeshEntityFactory::getEntityComponentTypes() const
{
	return makeTypeInfoSet< MeshComponentData >();
}

Ref< world::Entity > MeshEntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	return nullptr;
}

Ref< world::IEntityEvent > MeshEntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return nullptr;
}

Ref< world::IEntityComponent > MeshEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	const MeshComponentData* meshComponentData = mandatory_non_null_type_cast< const MeshComponentData* >(&entityComponentData);
	return meshComponentData->createComponent(m_resourceManager, m_renderSystem);
}

}
