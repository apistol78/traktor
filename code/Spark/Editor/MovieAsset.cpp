#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spark/Movie.h"
#include "Spark/Editor/MovieAsset.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.MovieAsset", 0, MovieAsset, editor::Asset)

MovieAsset::MovieAsset()
:	m_staticMovie(false)
,	m_includeAS(true)
{
}

void MovieAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	s >> Member< bool >(L"staticMovie", m_staticMovie);
	s >> Member< bool >(L"includeAS", m_includeAS);
}

	}
}
