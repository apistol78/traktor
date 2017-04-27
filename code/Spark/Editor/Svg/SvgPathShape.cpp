/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Spark/Editor/Svg/SvgPathShape.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SvgPathShape", SvgPathShape, SvgShape)

SvgPathShape::SvgPathShape(const Path& path)
:	m_path(path)
{
}

const Path& SvgPathShape::getPath() const
{
	return m_path;
}

	}
}
