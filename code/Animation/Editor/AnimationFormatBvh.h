#ifndef traktor_animation_AnimationFormatBvh_H
#define traktor_animation_AnimationFormatBvh_H

#include "Animation/Editor/IAnimationFormat.h"

namespace traktor
{
	namespace animation
	{

/*! \brief
 * \ingroup Animation
 */
class AnimationFormatBvh : public IAnimationFormat
{
	T_RTTI_CLASS;

public:
	virtual Ref< Animation > import(IStream* stream, const Vector4& offset, bool invertX, bool invertZ, bool autoCenterKeyPoses) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_animation_AnimationFormatBvh_H
