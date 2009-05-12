#include "Render/Dx10/Platform.h"
#include "Render/Dx10/VolumeTextureDx10.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VolumeTextureDx10", VolumeTextureDx10, VolumeTexture)

void VolumeTextureDx10::destroy()
{
}

int VolumeTextureDx10::getWidth() const
{
	return 0;
}

int VolumeTextureDx10::getHeight() const
{
	return 0;
}

int VolumeTextureDx10::getDepth() const
{
	return 0;
}

	}
}
