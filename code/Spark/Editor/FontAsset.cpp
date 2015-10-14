#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spark/Editor/FontAsset.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.FontAsset", 0, FontAsset, editor::Asset)

void FontAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	s >> Member< std::wstring >(L"includeCharacters", m_includeCharacters);
}

	}
}
