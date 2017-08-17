/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
#include "Render/Capture/Error.h"
#include "Render/Capture/SimpleTextureCapture.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SimpleTextureCapture", SimpleTextureCapture, ISimpleTexture)

SimpleTextureCapture::SimpleTextureCapture(ISimpleTexture* texture)
:	m_texture(texture)
,	m_locked(-1)
{
}

void SimpleTextureCapture::destroy()
{
	T_CAPTURE_ASSERT (m_texture, L"Simple texture already destroyed.");
	safeDestroy(m_texture);
}

ITexture* SimpleTextureCapture::resolve()
{
	T_CAPTURE_ASSERT (m_texture, L"Simple texture destroyed.");
	return this;
}

int SimpleTextureCapture::getWidth() const
{
	T_CAPTURE_ASSERT (m_texture, L"Simple texture destroyed.");
	return m_texture ? m_texture->getWidth() : 0;
}
	
int SimpleTextureCapture::getHeight() const
{
	T_CAPTURE_ASSERT (m_texture, L"Simple texture destroyed.");
	return m_texture ? m_texture->getHeight() : 0;
}
	
bool SimpleTextureCapture::lock(int level, Lock& lock)
{
	T_CAPTURE_ASSERT (m_texture, L"Simple texture destroyed.");
	T_CAPTURE_ASSERT (level >= 0, L"Invalid mip level.");
	T_CAPTURE_ASSERT (m_locked < 0, L"Already locked.");
	return m_texture ? m_texture->lock(level, lock) : false;
}

void SimpleTextureCapture::unlock(int level)
{
	T_CAPTURE_ASSERT (m_texture, L"Simple texture destroyed.");
	T_CAPTURE_ASSERT (level >= 0, L"Invalid mip level.");
	T_CAPTURE_ASSERT (m_locked != level, L"Trying to unlock incorrect mip level.");
	if (m_texture)
		m_texture->unlock(level);
	m_locked = -1;
}

void* SimpleTextureCapture::getInternalHandle()
{
	T_CAPTURE_ASSERT (m_texture, L"Simple texture destroyed.");
	return m_texture ? m_texture->getInternalHandle() : 0;
}

	}
}
