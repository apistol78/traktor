#ifndef traktor_animation_ISkeletonFormat_H
#define traktor_animation_ISkeletonFormat_H

#include "Core/Object.h"

namespace traktor
{

class IStream;

	namespace animation
	{

class Skeleton;

class ISkeletonFormat : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< Skeleton > import(IStream* stream) const = 0;
};

	}
}

#endif	// traktor_animation_ISkeletonFormat_H
