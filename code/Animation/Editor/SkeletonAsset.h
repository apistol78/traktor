#ifndef traktor_animation_SkeletonAsset_H
#define traktor_animation_SkeletonAsset_H

#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

class T_DLLCLASS SkeletonAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfo* getOutputType() const;
};

	}
}

#endif	// traktor_animation_SkeletonAsset_H
