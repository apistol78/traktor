/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
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
	T_FATAL_ASSERT_M (m_texture, L"Render error: Volume texture already destroyed.");
	safeDestroy(m_texture);
}

ITexture* VolumeTextureCapture::resolve()
{
	T_FATAL_ASSERT_M (m_texture, L"Render error: Volume texture destroyed.");
	return this;
}

int VolumeTextureCapture::getWidth() const
{
	T_FATAL_ASSERT_M (m_texture, L"Render error: Volume texture destroyed.");
	return m_texture->getWidth();
}
	
int VolumeTextureCapture::getHeight() const
{
	T_FATAL_ASSERT_M (m_texture, L"Render error: Volume texture destroyed.");
	return m_texture->getHeight();
}
	
int VolumeTextureCapture::getDepth() const
{
	T_FATAL_ASSERT_M (m_texture, L"Render error: Volume texture destroyed.");
	return m_texture->getDepth();
}

	}
}
