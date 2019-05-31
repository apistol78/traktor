#include "Ai/NavMesh.h"
#include "Ai/NavMeshComponent.h"
#include "Ai/NavMeshComponentData.h"
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
	return TypeInfoSet();
}

const TypeInfoSet NavMeshEntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet NavMeshEntityFactory::getEntityComponentTypes() const
{
	return makeTypeInfoSet< NavMeshComponentData >();
}

Ref< world::Entity > NavMeshEntityFactory::createEntity(
	const world::IEntityBuilder* builder,
	const world::EntityData& entityData
) const
{
	return nullptr;
}

Ref< world::IEntityEvent > NavMeshEntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return nullptr;
}

Ref< world::IEntityComponent > NavMeshEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	if (!m_editor)
	{
		auto navMeshComponentData = checked_type_cast< const NavMeshComponentData* >(&entityComponentData);

		resource::Proxy< NavMesh > navMesh;
		if (!m_resourceManager->bind(navMeshComponentData->get(), navMesh))
			return nullptr;

		return new NavMeshComponent(navMesh);
	}
	else
		return new NavMeshComponent();
}

	}
}
