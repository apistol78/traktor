#include <algorithm>
#include "World/IEntityRenderer.h"
#include "World/WorldEntityRenderers.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

/*! Create map between entities and it's associated entity renderer. */
void updateEntityRendererMap(
	const RefArray< IEntityRenderer >& entityRenderers,
	WorldEntityRenderers::entity_renderer_map_t& outEntityRendererMap
)
{
	outEntityRendererMap.clear();
	for (const auto entityRenderer : entityRenderers)
	{
		TypeInfoSet entityTypes = entityRenderer->getRenderableTypes();
		for (const auto entityType : entityTypes)
		{
			TypeInfoSet renderableTypes;
			entityType->findAllOf(renderableTypes);
			for (const auto renderableType : renderableTypes)
				outEntityRendererMap[renderableType] = entityRenderer;
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
