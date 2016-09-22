#ifndef traktor_animation_SkeletonFormatFbx_H
#define traktor_animation_SkeletonFormatFbx_H

#include "Animation/Editor/ISkeletonFormat.h"

namespace traktor
{
	namespace animation
	{

/*! \brief
 * \ingroup Animation
 */
class SkeletonFormatFbx : public ISkeletonFormat
{
	T_RTTI_CLASS;

public:
	virtual Ref< Skeleton > import(IStream* stream, const Vector4& offset, float scale, float radius, bool invertX, bool invertZ) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_animation_SkeletonFormatFbx_H
