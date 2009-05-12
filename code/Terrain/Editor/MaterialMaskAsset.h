#ifndef traktor_terrain_MaterialMaskAsset_H
#define traktor_terrain_MaterialMaskAsset_H

#include "Editor/Asset.h"

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

class T_DLLCLASS MaterialMaskAsset : public editor::Asset
{
	T_RTTI_CLASS(MaterialMaskAsset)

public:
	virtual const Type* getOutputType() const;
};

	}
}

#endif	// traktor_terrain_MaterialMaskAsset_H
