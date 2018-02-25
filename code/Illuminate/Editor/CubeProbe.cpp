/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Drawing/Image.h"
#include "Illuminate/Editor/CubeProbe.h"
#include "Render/Editor/Texture/CubeMap.h"

namespace traktor
{
	namespace illuminate
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.illuminate.CubeProbe", CubeProbe, IProbe)

CubeProbe::CubeProbe(const drawing::Image* cubeMap)
{
	Ref< drawing::Image > cubeMap4f = cubeMap->clone();
	cubeMap4f->convert(drawing::PixelFormat::getARGBF32());
	m_cubeMap = new render::CubeMap(cubeMap4f);
}

Color4f CubeProbe::sample(const Vector4& direction) const
{
	return m_cubeMap->get(direction);
}

	}
}
