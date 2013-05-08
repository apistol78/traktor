#ifndef traktor_hf_MaterialMaskAsset_H
#define traktor_hf_MaterialMaskAsset_H

#include "Core/RefArray.h"
#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
	{

class MaterialMaskAssetLayer;

class T_DLLCLASS MaterialMaskAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	const RefArray< MaterialMaskAssetLayer >& getLayers() const;

	virtual void serialize(ISerializer& s);

private:
	RefArray< MaterialMaskAssetLayer > m_layers;
};

	}
}

#endif	// traktor_hf_MaterialMaskAsset_H
