#include "Terrain/TerrainLayerComponent.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainLayerComponent", TerrainLayerComponent, world::IEntityComponent)

void TerrainLayerComponent::setOwner(world::Entity* owner)
{
	m_dirty = true;
}

void TerrainLayerComponent::update(const world::UpdateParams& update)
{
	if (m_dirty)
	{
		m_dirty = false;
		updatePatches();
	}
}

}
