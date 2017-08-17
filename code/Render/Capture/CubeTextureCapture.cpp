/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
#include "Render/Capture/CubeTextureCapture.h"
#include "Render/Capture/Error.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CubeTextureCapture", CubeTextureCapture, ICubeTexture)

CubeTextureCapture::CubeTextureCapture(ICubeTexture* texture)
:	m_texture(texture)
{
	m_locked[0] =
	m_locked[1] = -1;
}

void CubeTextureCapture::destroy()
{

	T_CAPTURE_ASSERT (m_texture, L"Cube texture already destroyed.");
	safeDestroy(m_texture);
}

ITexture* CubeTextureCapture::resolve()
{
	T_CAPTURE_ASSERT (m_texture, L"Cube texture destroyed.");
	return this;
}

int CubeTextureCapture::getWidth() const
{
	T_CAPTURE_ASSERT (m_texture, L"Cube texture destroyed.");
	return m_texture ? m_texture->getWidth() : 0;
}
	
int CubeTextureCapture::getHeight() const
{
	T_CAPTURE_ASSERT (m_texture, L"Cube texture destroyed.");
	return m_texture ? m_texture->getHeight() : 0;
}
	
int CubeTextureCapture::getDepth() const
{
	T_CAPTURE_ASSERT (m_texture, L"Cube texture destroyed.");
	return m_texture ? m_texture->getDepth() : 0;
}

bool CubeTextureCapture::lock(int side, int level, Lock& lock)
{
	T_CAPTURE_ASSERT (m_texture, L"Cube texture destroyed.");
	T_CAPTURE_ASSERT (side >= 0, L"Invalid side index.");
	T_CAPTURE_ASSERT (m_locked[0] < 0, L"Already locked.");
	if (m_texture && m_texture->lock(side, level, lock))
	{
		m_locked[0] = side;
		m_locked[1] = level;
		return true;
	}
	else
		return false;
}

void CubeTextureCapture::unlock(int side, int level)
{
	T_CAPTURE_ASSERT (m_texture, L"Cube texture destroyed.");
	T_CAPTURE_ASSERT (side >= 0, L"Invalid side index.");
	T_CAPTURE_ASSERT (m_locked[0] == side, L"Trying to unlock incorrect side.");
	T_CAPTURE_ASSERT (m_locked[1] == level, L"Trying to unlock incorrect level.");
	if (m_texture)
		m_texture->unlock(side, level);
	m_locked[0] =
	m_locked[1] = -1;
}

	}
}
