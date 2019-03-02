#include "Core/Misc/SafeDestroy.h"
#include "Render/Capture/Error.h"
#include "Render/Capture/VolumeTextureCapture.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VolumeTextureCapture", VolumeTextureCapture, IVolumeTexture)

VolumeTextureCapture::VolumeTextureCapture(IVolumeTexture* texture)
:	m_texture(texture)
{
}

void VolumeTextureCapture::destroy()
{
	T_CAPTURE_ASSERT (m_texture, L"Volume texture already destroyed.");
	safeDestroy(m_texture);
}

ITexture* VolumeTextureCapture::resolve()
{
	T_CAPTURE_ASSERT (m_texture, L"Volume texture destroyed.");
	return this;
}

int32_t VolumeTextureCapture::getMips() const
{
	T_CAPTURE_ASSERT (m_texture, L"Volume texture destroyed.");
	return m_texture ? m_texture->getMips() : 0;
}

int32_t VolumeTextureCapture::getWidth() const
{
	T_CAPTURE_ASSERT (m_texture, L"Volume texture destroyed.");
	return m_texture ? m_texture->getWidth() : 0;
}

int32_t VolumeTextureCapture::getHeight() const
{
	T_CAPTURE_ASSERT (m_texture, L"Volume texture destroyed.");
	return m_texture ? m_texture->getHeight() : 0;
}

int32_t VolumeTextureCapture::getDepth() const
{
	T_CAPTURE_ASSERT (m_texture, L"Volume texture destroyed.");
	return m_texture ? m_texture->getDepth() : 0;
}

	}
}
