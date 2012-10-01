#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Flash/FlashMovie.h"
#include "Flash/Editor/FlashMovieAsset.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.flash.FlashMovieAsset", 1, FlashMovieAsset, editor::Asset)

bool FlashMovieAsset::serialize(ISerializer& s)
{
	if (!editor::Asset::serialize(s))
		return false;

	if (s.getVersion() >= 1)
		s >> MemberStlVector< Guid >(L"dependentMovies", m_dependentMovies);

	return true;
}

	}
}
