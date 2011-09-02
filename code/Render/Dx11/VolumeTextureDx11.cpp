#include "Render/Dx11/Platform.h"
#include "Render/Dx11/VolumeTextureDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VolumeTextureDx11", VolumeTextureDx11, IVolumeTexture)

void VolumeTextureDx11::destroy()
{
}

ITexture* VolumeTextureDx11::resolve()
{
	return this;
}

int VolumeTextureDx11::getWidth() const
{
	return 0;
}

int VolumeTextureDx11::getHeight() const
{
	return 0;
}

int VolumeTextureDx11::getDepth() const
{
	return 0;
}

	}
}
