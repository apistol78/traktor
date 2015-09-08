#include "Render/Shader.h"
#include "Resource/Member.h"
#include "Spark/Editor/ShapeAsset.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.ShapeAsset", 0, ShapeAsset, editor::Asset)

void ShapeAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	s >> resource::Member< render::Shader >(L"shader", m_shader);
}

	}
}
