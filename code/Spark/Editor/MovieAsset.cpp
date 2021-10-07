#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spark/Movie.h"
#include "Spark/Editor/MovieAsset.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.MovieAsset", 1, MovieAsset, editor::Asset)

MovieAsset::MovieAsset()
:	m_staticMovie(false)
{
}

void MovieAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	s >> Member< bool >(L"staticMovie", m_staticMovie);

	if (s.getVersion< MovieAsset >() < 1)
	{
		bool includeAS;
		s >> Member< bool >(L"includeAS", includeAS);
	}
}

	}
}
