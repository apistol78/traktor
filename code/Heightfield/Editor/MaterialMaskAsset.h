#ifndef traktor_hf_MaterialMaskAsset_H
#define traktor_hf_MaterialMaskAsset_H

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

class T_DLLCLASS MaterialMaskAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfo* getOutputType() const;
};

	}
}

#endif	// traktor_hf_MaterialMaskAsset_H
