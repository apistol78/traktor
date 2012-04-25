#ifndef traktor_terrain_Terrain_H
#define traktor_terrain_Terrain_H

#include "Core/Object.h"
#include "Resource/Proxy.h"

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

class Terrain : public Object
{
	T_RTTI_CLASS;

public:
	const resource::Proxy< hf::Heightfield >& getHeightfield() const { return m_heightfield; }

	const resource::Proxy< render::ISimpleTexture >& getNormalMap() const { return m_normalMap; }

	const resource::Proxy< render::ISimpleTexture >& getHeightMap() const { return m_heightMap; }

	const std::vector< resource::Proxy< render::Shader > >& getSurfaceLayers() const { return m_surfaceLayers; }

private:
	friend class TerrainFactory;

	resource::Proxy< hf::Heightfield > m_heightfield;
	resource::Proxy< render::ISimpleTexture > m_normalMap;
	resource::Proxy< render::ISimpleTexture > m_heightMap;
	std::vector< resource::Proxy< render::Shader > > m_surfaceLayers;
};

	}
}

#endif	// traktor_terrain_Terrain_H
