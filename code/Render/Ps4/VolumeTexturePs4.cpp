#include "Render/Ps4/VolumeTexturePs4.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VolumeTexturePs4", VolumeTexturePs4, IVolumeTexture)

VolumeTexturePs4::VolumeTexturePs4(ContextPs4* context)
:	m_context(context)
{
}

VolumeTexturePs4::~VolumeTexturePs4()
{
	destroy();
}

bool VolumeTexturePs4::create(const VolumeTextureCreateDesc& desc)
{
	return true;
}

void VolumeTexturePs4::destroy()
{
}

ITexture* VolumeTexturePs4::resolve()
{
	return this;
}

int VolumeTexturePs4::getWidth() const
{
	return 0;
}

int VolumeTexturePs4::getHeight() const
{
	return 0;
}

int VolumeTexturePs4::getDepth() const
{
	return 0;
}

	}
}
