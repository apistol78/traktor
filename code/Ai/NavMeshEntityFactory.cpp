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

NavMeshEntityFactory::NavMeshEntityFactory(resource::IResourceManager* resourceManager, bool editor)
:	m_resourceManager(resourceManager)
,	m_editor(editor)
{
}

const TypeInfoSet NavMeshEntityFactory::getEntityTypes() const
{
	return makeTypeInfoSet< NavMeshEntityData >();
}

const TypeInfoSet NavMeshEntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet NavMeshEntityFactory::getEntityComponentTypes() const
{
	return TypeInfoSet();
}

Ref< world::Entity > NavMeshEntityFactory::createEntity(
	const world::IEntityBuilder* builder,
	const world::EntityData& entityData
) const
{
	if (!m_editor)
	{
		const NavMeshEntityData* navMeshEntityData = checked_type_cast< const NavMeshEntityData* >(&entityData);

		resource::Proxy< NavMesh > navMesh;
		if (!m_resourceManager->bind(navMeshEntityData->get(), navMesh))
			return nullptr;

		return new NavMeshEntity(navMesh);
	}
	else
		return new NavMeshEntity();
}

Ref< world::IEntityEvent > NavMeshEntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return nullptr;
}

Ref< world::IEntityComponent > NavMeshEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	return nullptr;
}

	}
}
