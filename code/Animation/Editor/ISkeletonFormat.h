/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_animation_ISkeletonFormat_H
#define traktor_animation_ISkeletonFormat_H

#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{

class IStream;
class Vector4;

	namespace animation
	{

class Skeleton;

/*! \brief
 * \ingroup Animation
 */
class ISkeletonFormat : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< Skeleton > import(IStream* stream, const Vector4& offset, float scale, float radius, bool invertX, bool invertZ) const = 0;
};

	}
}

#endif	// traktor_animation_ISkeletonFormat_H
