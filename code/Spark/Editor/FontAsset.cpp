#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "Spark/Editor/FontAsset.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.FontAsset", 0, FontAsset, editor::Asset)

void FontAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> Member< std::wstring >(L"includeCharacters", m_includeCharacters);
}

	}
}
