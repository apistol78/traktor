#include "Flash/Editor/FlashMovieAsset.h"
#include "Flash/FlashMovie.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.flash.FlashMovieAsset", FlashMovieAsset, editor::Asset)

const TypeInfo* FlashMovieAsset::getOutputType() const
{
	return &type_of< FlashMovie >();
}

	}
}
