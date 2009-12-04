#include "Animation/Animation/Animation.h"
#include "Animation/Editor/AnimationAsset.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.AnimationAsset", 0, AnimationAsset, editor::Asset)

const TypeInfo* AnimationAsset::getOutputType() const
{
	return &type_of< Animation >();
}

	}
}
