#include "Mesh/AbstractMeshEntityData.h"
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

Ref< world::Entity > MeshEntityFactory::createEntity(world::IEntityBuilder* builder, const std::wstring& name, const world::EntityData& entityData, const Object* instanceData) const
{
	const AbstractMeshEntityData* meshEntityData = checked_type_cast< const AbstractMeshEntityData* >(&entityData);
	return meshEntityData->createEntity(m_resourceManager, builder);
}

	}
}
