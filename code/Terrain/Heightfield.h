#ifndef traktor_terrain_Heightfield_H
#define traktor_terrain_Heightfield_H

#include "Terrain/HeightfieldResource.h"

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

class ITexture;

	}

	namespace terrain
	{

class T_DLLCLASS Heightfield : public Object
{
	T_RTTI_CLASS;

public:
	typedef unsigned short height_t;

	Heightfield(const HeightfieldResource& resource);

	virtual ~Heightfield();

	float getGridHeight(float gridX, float gridZ) const;

	float getWorldHeight(float worldX, float worldZ) const;

	const height_t* getHeights() const;

	void setHeightTexture(render::ITexture* heightTexture);

	void setNormalTexture(render::ITexture* normalTexture);

	Ref< render::ITexture > getHeightTexture() const;

	Ref< render::ITexture > getNormalTexture() const;

	inline const HeightfieldResource& getResource() const { return m_resource; }

private:
	HeightfieldResource m_resource;
	height_t* m_heights;
	Ref< render::ITexture > m_heightTexture;
	Ref< render::ITexture > m_normalTexture;
};

	}
}

#endif	// traktor_terrain_Heightfield_H
