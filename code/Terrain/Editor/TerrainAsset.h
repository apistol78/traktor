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

class ISimpleTexture;
class Shader;

	}

	namespace terrain
	{

class T_DLLCLASS TerrainAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	TerrainAsset();

	virtual bool serialize(ISerializer& s);

	uint32_t getDetailSkip() const { return m_detailSkip; }

	uint32_t getPatchDim() const { return m_patchDim; }

	const resource::Id< hf::Heightfield >& getHeightfield() const { return m_heightfield; }

	const resource::Id< render::ISimpleTexture >& getSplatMap() const { return m_splatMap; }

	const resource::Id< render::Shader >& getSurfaceShader() const { return m_surfaceShader; }

private:
	uint32_t m_detailSkip;
	uint32_t m_patchDim;
	resource::Id< hf::Heightfield > m_heightfield;
	resource::Id< render::ISimpleTexture > m_splatMap;
	resource::Id< render::Shader > m_surfaceShader;
};

	}
}

#endif	// traktor_terrain_TerrainAsset_H
