#ifndef traktor_terrain_HeightfieldAsset_H
#define traktor_terrain_HeightfieldAsset_H

#include "Editor/Asset.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace terrain
	{

class T_DLLCLASS HeightfieldAsset : public editor::Asset
{
	T_RTTI_CLASS(HeightfieldAsset)

public:
	HeightfieldAsset();

	virtual const Type* getOutputType() const;

	virtual bool serialize(Serializer& s);

private:
	friend class HeightfieldPipeline;

	Vector4 m_worldExtent;
	uint32_t m_patchDim;
	uint32_t m_detailSkip;
};

	}
}

#endif	// traktor_terrain_HeightfieldAsset_H
