#include "Ai/NavMesh.h"
#include "Ai/NavMeshEntity.h"
#include "Ai/NavMeshEntityData.h"
#include "Ai/NavMeshEntityFactory.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace ai
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ai.NavMeshEntityFactory", NavMeshEntityFactory, world::IEntityFactory)

NavMeshEntityFactory::NavMeshEntityFactory(resource::IResourceManager* resourceManager)
:	m_resourceManager(resourceManager)
{
}

const TypeInfoSet NavMeshEntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< NavMeshEntityData >());
	return typeSet;
}

Ref< world::Entity > NavMeshEntityFactory::createEntity(
	const world::IEntityBuilder* builder,
	const world::EntityData& entityData
) const
{
	const NavMeshEntityData* navMeshEntityData = checked_type_cast< const NavMeshEntityData* >(&entityData);

	resource::Proxy< NavMesh > navMesh;
	if (!m_resourceManager->bind(navMeshEntityData->get(), navMesh))
		return 0;

	return new NavMeshEntity(navMesh);
}

	}
}
