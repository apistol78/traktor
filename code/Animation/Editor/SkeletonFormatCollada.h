#ifndef traktor_animation_SkeletonFormatCollada_H
#define traktor_animation_SkeletonFormatCollada_H

#include "Animation/Editor/ISkeletonFormat.h"

namespace traktor
{
	namespace animation
	{

//class BvhDocument;

class SkeletonFormatCollada : public ISkeletonFormat
{
	T_RTTI_CLASS;

public:
//	Ref< Skeleton > create(const BvhDocument* document) const;

	virtual Ref< Skeleton > import(IStream* stream) const;
};

	}
}

#endif	// traktor_animation_SkeletonFormatCollada_H
