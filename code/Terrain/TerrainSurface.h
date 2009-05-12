#ifndef traktor_terrain_TerrainSurface_H
#define traktor_terrain_TerrainSurface_H

#include "Core/Serialization/Serializable.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Shader;

	}

	namespace terrain
	{

class T_DLLCLASS TerrainSurface : public Serializable
{
	T_RTTI_CLASS(TerrainSurface)

public:
	inline std::vector< resource::Proxy< render::Shader > >& getLayers() { return m_layers; }

	virtual bool serialize(Serializer& s);

private:
	std::vector< resource::Proxy< render::Shader > > m_layers;
};

	}
}

#endif	// traktor_terrain_TerrainSurface_H
