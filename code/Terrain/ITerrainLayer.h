#ifndef traktor_terrain_ITerrainLayer_H
#define traktor_terrain_ITerrainLayer_H

#include "Core/Object.h"

namespace traktor
{
	namespace world
	{

class IWorldRenderPass;
struct UpdateParams;
class WorldContext;
class WorldRenderView;

	}

	namespace terrain
	{

class TerrainComponent;

class ITerrainLayer : public Object
{
	T_RTTI_CLASS;

public:
	virtual void update(const world::UpdateParams& update) = 0;

	virtual void render(
		TerrainComponent& terrainComponent,
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	) = 0;

	virtual void updatePatches(const TerrainComponent& terrainComponent) = 0;
};

	}
}

#endif	// traktor_terrain_ITerrainLayer_H
