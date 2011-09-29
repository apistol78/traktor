#ifndef traktor_hf_Heightfield_H
#define traktor_hf_Heightfield_H

#include "Heightfield/HeightfieldTypes.h"
#include "Heightfield/HeightfieldResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
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

	inline const HeightfieldResource& getResource() const { return m_resource; }

private:
	HeightfieldResource m_resource;
	height_t* m_heights;
};

	}
}

#endif	// traktor_hf_Heightfield_H
