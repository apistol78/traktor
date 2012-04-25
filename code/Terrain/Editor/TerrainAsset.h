#ifndef traktor_terrain_TerrainAsset_H
#define traktor_terrain_TerrainAsset_H

#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
	{

class Heightfield;
class MaterialMask;

	}

	namespace render
	{

class Shader;

	}

	namespace terrain
	{

class T_DLLCLASS TerrainAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual bool serialize(ISerializer& s);

	const resource::Id< hf::Heightfield >& getHeightfield() const { return m_heightfield; }

	const std::vector< resource::Id< render::Shader > >& getSurfaceLayers() const { return m_surfaceLayers; }

private:
	resource::Id< hf::Heightfield > m_heightfield;
	std::vector< resource::Id< render::Shader > > m_surfaceLayers;
};

	}
}

#endif	// traktor_terrain_TerrainAsset_H
