#include "Terrain/TerrainLayerComponentData.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainLayerComponentData", TerrainLayerComponentData, world::IEntityComponentData)

void TerrainLayerComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

}
