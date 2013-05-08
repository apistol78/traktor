#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Flash/FlashMovie.h"
#include "Flash/Editor/FlashMovieAsset.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.flash.FlashMovieAsset", 1, FlashMovieAsset, editor::Asset)

void FlashMovieAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	if (s.getVersion() >= 1)
		s >> MemberStlVector< Guid >(L"dependentMovies", m_dependentMovies);
}

	}
}
