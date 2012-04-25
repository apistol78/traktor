#ifndef traktor_terrain_TerrainResource_H
#define traktor_terrain_TerrainResource_H

#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
	{

class Heightfield;

	}

	namespace render
	{

class ISimpleTexture;
class Shader;

	}

	namespace terrain
	{

class T_DLLCLASS TerrainResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual bool serialize(ISerializer& s);

	const resource::Id< hf::Heightfield >& getHeightfield() const { return m_heightfield; }

	const resource::Id< render::ISimpleTexture >& getNormalMap() const { return m_normalMap; }

	const resource::Id< render::ISimpleTexture >& getHeightMap() const { return m_heightMap; }

	const std::vector< resource::Id< render::Shader > >& getSurfaceLayers() const { return m_surfaceLayers; }

private:
	friend class TerrainPipeline;

	resource::Id< hf::Heightfield > m_heightfield;
	resource::Id< render::ISimpleTexture > m_normalMap;
	resource::Id< render::ISimpleTexture > m_heightMap;
	std::vector< resource::Id< render::Shader > > m_surfaceLayers;
};

	}
}

#endif	// traktor_terrain_TerrainResource_H
