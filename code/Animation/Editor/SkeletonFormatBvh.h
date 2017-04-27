/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_animation_SkeletonFormatBvh_H
#define traktor_animation_SkeletonFormatBvh_H

#include "Animation/Editor/ISkeletonFormat.h"

namespace traktor
{
	namespace animation
	{

class BvhDocument;

/*! \brief
 * \ingroup Animation
 */
class SkeletonFormatBvh : public ISkeletonFormat
{
	T_RTTI_CLASS;

public:
	Ref< Skeleton > create(const BvhDocument* document, const Vector4& offset, float radius) const;

	virtual Ref< Skeleton > import(IStream* stream, const Vector4& offset, float scale, float radius, bool invertX, bool invertZ) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_animation_SkeletonFormatBvh_H
