#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "Spark/Editor/ShapeAsset.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.ShapeAsset", 0, ShapeAsset, editor::Asset)

ShapeAsset::ShapeAsset()
:	m_cubicApproximationError(1.0f)
,	m_pivot(PtViewTopLeft)
{
}

void ShapeAsset::serialize(ISerializer& s)
{
	const MemberEnum< PivotType >::Key c_pivotTypeKeys[] =
	{
		{ L"PtViewTopLeft", PtViewTopLeft },
		{ L"PtViewCenter", PtViewCenter },
		{ L"PtShapeCenter", PtShapeCenter },
		{ 0 }
	};

	editor::Asset::serialize(s);

	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> Member< float >(L"cubicApproximationError", m_cubicApproximationError, AttributeRange(0.0f));
	s >> MemberEnum< PivotType >(L"pivot", m_pivot, c_pivotTypeKeys);
}

	}
}
