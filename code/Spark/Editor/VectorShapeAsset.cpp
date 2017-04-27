/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberEnum.h"
#include "Spark/Editor/VectorShapeAsset.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.VectorShapeAsset", 0, VectorShapeAsset, ShapeAsset)

VectorShapeAsset::VectorShapeAsset()
:	m_pivot(PtViewTopLeft)
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

	s >> MemberEnum< PivotType >(L"pivot", m_pivot, c_pivotTypeKeys);
}

	}
}
