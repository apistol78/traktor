#include "Core/Misc/SafeDestroy.h"
#include "Render/Capture/CubeTextureCapture.h"

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
	T_FATAL_ASSERT_M (m_texture, L"Render error: Cube texture already destroyed.");
	safeDestroy(m_texture);
}

ITexture* CubeTextureCapture::resolve()
{
	T_FATAL_ASSERT_M (m_texture, L"Render error: Cube texture destroyed.");
	return this;
}

int CubeTextureCapture::getWidth() const
{
	T_FATAL_ASSERT_M (m_texture, L"Render error: Cube texture destroyed.");
	return m_texture->getWidth();
}
	
int CubeTextureCapture::getHeight() const
{
	T_FATAL_ASSERT_M (m_texture, L"Render error: Cube texture destroyed.");
	return m_texture->getHeight();
}
	
int CubeTextureCapture::getDepth() const
{
	T_FATAL_ASSERT_M (m_texture, L"Render error: Cube texture destroyed.");
	return m_texture->getDepth();
}

bool CubeTextureCapture::lock(int side, int level, Lock& lock)
{
	T_FATAL_ASSERT_M (m_texture, L"Render error: Cube texture destroyed.");
	T_FATAL_ASSERT_M (side >= 0, L"Render error: Invalid side index.");
	T_FATAL_ASSERT_M (m_locked[0] < 0, L"Render error: Already locked.");
	if (m_texture->lock(side, level, lock))
	{
		m_locked[0] = side;
		m_locked[1] = level;
	}
	else
		return false;
}

void CubeTextureCapture::unlock(int side, int level)
{
	T_FATAL_ASSERT_M (m_texture, L"Render error: Cube texture destroyed.");
	T_FATAL_ASSERT_M (side >= 0, L"Render error: Invalid side index.");
	T_FATAL_ASSERT_M (m_locked[0] == side, L"Render error: Trying to unlock incorrect side.");
	T_FATAL_ASSERT_M (m_locked[1] == level, L"Render error: Trying to unlock incorrect level.");
	m_texture->unlock(side, level);
	m_locked[0] =
	m_locked[1] = -1;
}

	}
}
