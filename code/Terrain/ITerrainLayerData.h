#ifndef traktor_terrain_ITerrainLayerData_H
#define traktor_terrain_ITerrainLayerData_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class Shader;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace terrain
	{

class ITerrainLayer;
class TerrainEntity;

class T_DLLCLASS ITerrainLayerData : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< ITerrainLayer > createLayerInstance(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const TerrainEntity& terrainEntity
	) const = 0;
};

	}
}

#endif	// traktor_terrain_ITerrainLayerData_H
