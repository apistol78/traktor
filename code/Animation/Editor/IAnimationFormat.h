#ifndef traktor_animation_IAnimationFormat_H
#define traktor_animation_IAnimationFormat_H

#include "Core/Object.h"

namespace traktor
{

class IStream;

	namespace animation
	{

class Animation;

class IAnimationFormat : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< Animation > import(IStream* stream) const = 0;
};

	}
}

#endif	// traktor_animation_IAnimationFormat_H
