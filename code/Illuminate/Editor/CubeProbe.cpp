/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Illuminate/Editor/CubeProbe.h"
#include "Render/Editor/Texture/CubeMap.h"

namespace traktor
{
	namespace illuminate
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.illuminate.CubeProbe", CubeProbe, IProbe)

CubeProbe::CubeProbe(const render::CubeMap* cubeMap)
:	m_cubeMap(cubeMap)
{
}

Color4f CubeProbe::sample(const Vector4& direction) const
{
	return m_cubeMap->get(direction);
}

	}
}
