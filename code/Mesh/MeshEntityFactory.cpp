#include "Mesh/MeshEntityFactory.h"
#include "Mesh/MeshEntityData.h"
#include "Mesh/MeshEntity.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshEntityFactory", MeshEntityFactory, world::EntityFactory)

MeshEntityFactory::MeshEntityFactory(resource::IResourceManager* resourceManager)
:	m_resourceManager(resourceManager)
{
}

const TypeSet MeshEntityFactory::getEntityTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< MeshEntityData >());
	return typeSet;
}

world::Entity* MeshEntityFactory::createEntity(world::EntityBuilder* builder, const world::EntityData& entityData) const
{
	return checked_type_cast< const MeshEntityData* >(&entityData)->createEntity(m_resourceManager, builder);
}

	}
}
