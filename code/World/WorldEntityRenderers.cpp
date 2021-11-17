#include <algorithm>
#include "World/IEntityRenderer.h"
#include "World/WorldEntityRenderers.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

/*! Find only direct child types. */
void findAllChildTypes(const TypeInfo* type, TypeInfoSet& outChildTypes)
{
	for (const auto descendantType : type->findAllOf(false))
	{
		if (descendantType->getSuper() == type)
			outChildTypes.insert(descendantType);
	}
}

/*! Fill entity render map until another "root" is found. */
void fillEntityRenderMap(const TypeInfo* renderableType, IEntityRenderer* entityRenderer, WorldEntityRenderers::entity_renderer_map_t& outEntityRendererMap)
{
	if (outEntityRendererMap[renderableType] != nullptr)
		return;

	outEntityRendererMap[renderableType] = entityRenderer;

	TypeInfoSet childTypes;
	findAllChildTypes(renderableType, childTypes);
	for (auto childType : childTypes)
		fillEntityRenderMap(childType, entityRenderer, outEntityRendererMap);
}

/*! Create map between entities and it's associated entity renderer. */
void updateEntityRendererMap(
	const RefArray< IEntityRenderer >& entityRenderers,
	WorldEntityRenderers::entity_renderer_map_t& outEntityRendererMap
)
{
	outEntityRendererMap.clear();

	// First insert all exact matching entity types, "roots".
	for (const auto entityRenderer : entityRenderers)
	{
		TypeInfoSet entityTypes = entityRenderer->getRenderableTypes();
		for (const auto entityType : entityTypes)
			outEntityRendererMap[entityType] = entityRenderer;
	}

	// Second pass we flood fill with derived entity types until we reach another "root".
	for (const auto entityRenderer : entityRenderers)
	{
		TypeInfoSet entityTypes = entityRenderer->getRenderableTypes();
		for (const auto entityType : entityTypes)
		{
			TypeInfoSet childTypes;
			findAllChildTypes(entityType, childTypes);
			for (const auto childType : childTypes)
				fillEntityRenderMap(childType, entityRenderer, outEntityRendererMap);
		}
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldEntityRenderers", WorldEntityRenderers, Object)

void WorldEntityRenderers::add(IEntityRenderer* entityRenderer)
{
	m_entityRenderers.push_back(entityRenderer);
	updateEntityRendererMap(m_entityRenderers, m_entityRendererMap);
}

void WorldEntityRenderers::remove(IEntityRenderer* entityRenderer)
{
	const auto i = std::find(m_entityRenderers.begin(), m_entityRenderers.end(), entityRenderer);
	T_ASSERT_M(i != m_entityRenderers.end(), L"No such entity renderer");
	m_entityRenderers.erase(i);
	updateEntityRendererMap(m_entityRenderers, m_entityRendererMap);
}

	}
}
