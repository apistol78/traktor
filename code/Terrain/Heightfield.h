#ifndef traktor_terrain_Heightfield_H
#define traktor_terrain_Heightfield_H

#include "Terrain/HeightfieldTypes.h"
#include "Terrain/HeightfieldResource.h"

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

class ISimpleTexture;

	}

	namespace terrain
	{

class T_DLLCLASS Heightfield : public Object
{
	T_RTTI_CLASS;

public:
	Heightfield(const HeightfieldResource& resource);

	virtual ~Heightfield();

	float getGridHeight(float gridX, float gridZ) const;

	float getWorldHeight(float worldX, float worldZ) const;

	void gridToWorld(float gridX, float gridZ, float& outWorldX, float& outWorldZ) const;

	void worldToGrid(float worldX, float worldZ, float& outGridX, float& outGridZ) const;

	const height_t* getHeights() const;

	void setHeightTexture(render::ISimpleTexture* heightTexture);

	void setNormalTexture(render::ISimpleTexture* normalTexture);

	render::ISimpleTexture* getHeightTexture() const;

	render::ISimpleTexture* getNormalTexture() const;

	inline const HeightfieldResource& getResource() const { return m_resource; }

private:
	HeightfieldResource m_resource;
	height_t* m_heights;
	Ref< render::ISimpleTexture > m_heightTexture;
	Ref< render::ISimpleTexture > m_normalTexture;
};

	}
}

#endif	// traktor_terrain_Heightfield_H
