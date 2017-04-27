/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_animation_IAnimationFormat_H
#define traktor_animation_IAnimationFormat_H

#include "Core/Object.h"

namespace traktor
{

class IStream;
class Vector4;

	namespace animation
	{

class Animation;

/*! \brief
 * \ingroup Animation
 */
class IAnimationFormat : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< Animation > import(IStream* stream, const Vector4& offset, bool invertX, bool invertZ, bool autoCenterKeyPoses) const = 0;
};

	}
}

#endif	// traktor_animation_IAnimationFormat_H
