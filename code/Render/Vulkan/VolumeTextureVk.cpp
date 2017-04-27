/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Vulkan/VolumeTextureVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VolumeTextureVk", VolumeTextureVk, IVolumeTexture)

VolumeTextureVk::VolumeTextureVk()
{
}

VolumeTextureVk::~VolumeTextureVk()
{
	destroy();
}

bool VolumeTextureVk::create(const VolumeTextureCreateDesc& desc)
{
	return true;
}

void VolumeTextureVk::destroy()
{
}

ITexture* VolumeTextureVk::resolve()
{
	return this;
}

int VolumeTextureVk::getWidth() const
{
	return 0;
}

int VolumeTextureVk::getHeight() const
{
	return 0;
}

int VolumeTextureVk::getDepth() const
{
	return 0;
}

	}
}
