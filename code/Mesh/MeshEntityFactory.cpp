#include "Mesh/MeshEntityFactory.h"
#include "Mesh/MeshEntityData.h"
#include "Mesh/MeshEntity.h"

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
	typeSet.insert(&type_of< MeshEntityData >());
	return typeSet;
}

Ref< world::Entity > MeshEntityFactory::createEntity(world::IEntityBuilder* builder, const std::wstring& name, const world::EntityData& entityData, const Object* instanceData) const
{
	if (const MeshEntityData* meshEntityData = dynamic_type_cast< const MeshEntityData* >(&entityData))
		return meshEntityData->createEntity(m_resourceManager, builder);
	else
	{
		T_BREAKPOINT;
		return 0;
	}
}

	}
}
