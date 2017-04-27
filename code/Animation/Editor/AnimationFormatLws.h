/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_animation_AnimationFormatLws_H
#define traktor_animation_AnimationFormatLws_H

#include "Animation/Editor/IAnimationFormat.h"

namespace traktor
{
	namespace animation
	{

/*! \brief
 * \ingroup Animation
 */
class AnimationFormatLws : public IAnimationFormat
{
	T_RTTI_CLASS;

public:
	virtual Ref< Animation > import(IStream* stream, const Vector4& offset, bool invertX, bool invertZ, bool autoCenterKeyPoses) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_animation_AnimationFormatLws_H
