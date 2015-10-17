#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Flash/FlashMovie.h"
#include "Flash/Editor/FlashMovieAsset.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.flash.FlashMovieAsset", 2, FlashMovieAsset, editor::Asset)

FlashMovieAsset::FlashMovieAsset()
:	m_staticMovie(false)
{
}

void FlashMovieAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	if (s.getVersion() == 1)
	{
		std::vector< Guid > dependentMovies;
		s >> MemberStlVector< Guid >(L"dependentMovies", dependentMovies);
	}

	if (s.getVersion() >= 2)
		s >> Member< bool >(L"staticMovie", m_staticMovie);
}

	}
}
