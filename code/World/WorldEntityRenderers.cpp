#include <algorithm>
#include "World/IEntityRenderer.h"
#include "World/WorldEntityRenderers.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

/*! \brief Create map between entities and it's associated entity renderer. */
void updateEntityRendererMap(
	const RefArray< IEntityRenderer >& entityRenderers,
	WorldEntityRenderers::entity_renderer_map_t& outEntityRendererMap
)
{
	outEntityRendererMap.clear();
	for (RefArray< IEntityRenderer >::const_iterator i = entityRenderers.begin(); i != entityRenderers.end(); ++i)
	{
		TypeInfoSet entityTypes = (*i)->getEntityTypes();
		for (TypeInfoSet::const_iterator j = entityTypes.begin(); j != entityTypes.end(); ++j)
		{
			TypeInfoSet renderableTypes;
			(*j)->findAllOf(renderableTypes);

			for (TypeInfoSet::const_iterator k = renderableTypes.begin(); k != renderableTypes.end(); ++k)
				outEntityRendererMap[*k] = *i;
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
	RefArray< IEntityRenderer >::iterator i = std::find(m_entityRenderers.begin(), m_entityRenderers.end(), entityRenderer);
	T_ASSERT_M (i != m_entityRenderers.end(), L"No such entity renderer");
	m_entityRenderers.erase(i);
	updateEntityRendererMap(m_entityRenderers, m_entityRendererMap);
}

IEntityRenderer* WorldEntityRenderers::find(const TypeInfo& entityType) const
{
	entity_renderer_map_t::const_iterator i = m_entityRendererMap.find(&entityType);
	return i != m_entityRendererMap.end() ? i->second : 0;
}

	}
}
