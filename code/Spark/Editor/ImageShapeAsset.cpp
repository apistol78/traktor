/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberEnum.h"
#include "Spark/Editor/ImageShapeAsset.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.ImageShapeAsset", 0, ImageShapeAsset, ShapeAsset)

ImageShapeAsset::ImageShapeAsset()
:	m_pivot(PtTopLeft)
{
}

void ImageShapeAsset::serialize(ISerializer& s)
{
	const MemberEnum< PivotType >::Key c_pivotTypeKeys[] =
	{
		{ L"PtTopLeft", PtTopLeft },
		{ L"PtCenter", PtCenter },
		{ 0 }
	};

	ShapeAsset::serialize(s);

	s >> MemberEnum< PivotType >(L"pivot", m_pivot, c_pivotTypeKeys);
}

	}
}
