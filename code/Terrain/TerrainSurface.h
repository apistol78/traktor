#ifndef traktor_terrain_TerrainSurface_H
#define traktor_terrain_TerrainSurface_H

#include "Core/Serialization/ISerializable.h"
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

class T_DLLCLASS TerrainSurface : public ISerializable
{
	T_RTTI_CLASS;

public:
	inline std::vector< resource::Proxy< render::Shader > >& getLayers() { return m_layers; }

	virtual bool serialize(ISerializer& s);

private:
	std::vector< resource::Proxy< render::Shader > > m_layers;
};

	}
}

#endif	// traktor_terrain_TerrainSurface_H
