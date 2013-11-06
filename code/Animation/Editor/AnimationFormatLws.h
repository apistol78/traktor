#ifndef traktor_animation_AnimationFormatLws_H
#define traktor_animation_AnimationFormatLws_H

#include "Animation/Editor/IAnimationFormat.h"

namespace traktor
{
	namespace animation
	{

class AnimationFormatLws : public IAnimationFormat
{
	T_RTTI_CLASS;

public:
	virtual Ref< Animation > import(IStream* stream, const Vector4& offset, bool invertX, bool invertZ) const;
};

	}
}

#endif	// traktor_animation_AnimationFormatLws_H
