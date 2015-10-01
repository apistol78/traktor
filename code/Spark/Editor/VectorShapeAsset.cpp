#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Spark/Editor/VectorShapeAsset.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.VectorShapeAsset", 0, VectorShapeAsset, ShapeAsset)

VectorShapeAsset::VectorShapeAsset()
:	m_cubicApproximationError(1.0f)
,	m_pivot(PtViewTopLeft)
{
}

void VectorShapeAsset::serialize(ISerializer& s)
{
	const MemberEnum< PivotType >::Key c_pivotTypeKeys[] =
	{
		{ L"PtViewTopLeft", PtViewTopLeft },
		{ L"PtViewCenter", PtViewCenter },
		{ L"PtShapeCenter", PtShapeCenter },
		{ 0 }
	};

	ShapeAsset::serialize(s);

	s >> Member< float >(L"cubicApproximationError", m_cubicApproximationError, AttributeRange(0.0f));
	s >> MemberEnum< PivotType >(L"pivot", m_pivot, c_pivotTypeKeys);
}

	}
}
