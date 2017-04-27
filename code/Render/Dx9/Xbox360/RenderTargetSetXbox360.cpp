/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Dx9/Platform.h"
#include "Render/Dx9/Xbox360/RenderTargetSetXbox360.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetXbox360", RenderTargetSetXbox360, RenderTargetSet)

void RenderTargetSetXbox360::destroy()
{
}

int RenderTargetSetXbox360::getWidth() const
{
	return 0;
}

int RenderTargetSetXbox360::getHeight() const
{
	return 0;
}

ISimpleTexture* RenderTargetSetXbox360::getColorTexture(int index) const
{
	return 0;
}

void RenderTargetSetXbox360::swap(int index1, int index2)
{
}

void RenderTargetSetXbox360::discard()
{
}

bool RenderTargetSetXbox360::read(int index, void* buffer) const
{
	return false;
}

	}
}
