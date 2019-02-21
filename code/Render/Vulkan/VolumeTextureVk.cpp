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

int32_t VolumeTextureVk::getMips() const
{
	return 0;
}

int32_t VolumeTextureVk::getWidth() const
{
	return 0;
}

int32_t VolumeTextureVk::getHeight() const
{
	return 0;
}

int32_t VolumeTextureVk::getDepth() const
{
	return 0;
}

	}
}
