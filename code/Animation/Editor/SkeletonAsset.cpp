#include "Animation/Editor/SkeletonAsset.h"
#include "Animation/Skeleton.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.SkeletonAsset", 0, SkeletonAsset, editor::Asset)

const TypeInfo* SkeletonAsset::getOutputType() const
{
	return &type_of< Skeleton >();
}

	}
}
