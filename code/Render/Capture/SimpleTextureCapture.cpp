#include "Core/Misc/SafeDestroy.h"
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
	T_FATAL_ASSERT_M (m_texture, L"Render error: Simple texture already destroyed.");
	safeDestroy(m_texture);
}

ITexture* SimpleTextureCapture::resolve()
{
	T_FATAL_ASSERT_M (m_texture, L"Render error: Simple texture destroyed.");
	return this;
}

int SimpleTextureCapture::getWidth() const
{
	T_FATAL_ASSERT_M (m_texture, L"Render error: Simple texture destroyed.");
	return m_texture->getWidth();
}
	
int SimpleTextureCapture::getHeight() const
{
	T_FATAL_ASSERT_M (m_texture, L"Render error: Simple texture destroyed.");
	return m_texture->getHeight();
}
	
bool SimpleTextureCapture::lock(int level, Lock& lock)
{
	T_FATAL_ASSERT_M (m_texture, L"Render error: Simple texture destroyed.");
	T_FATAL_ASSERT_M (level >= 0, L"Render error: Invalid mip level.");
	T_FATAL_ASSERT_M (m_locked < 0, L"Render error: Already locked.");
	return m_texture->lock(level, lock);
}

void SimpleTextureCapture::unlock(int level)
{
	T_FATAL_ASSERT_M (m_texture, L"Render error: Simple texture destroyed.");
	T_FATAL_ASSERT_M (level >= 0, L"Render error: Invalid mip level.");
	T_FATAL_ASSERT_M (m_locked != level, L"Render error: Trying to unlock incorrect mip level.");
	m_texture->unlock(level);
	m_locked = -1;
}

void* SimpleTextureCapture::getInternalHandle()
{
	T_FATAL_ASSERT_M (m_texture, L"Render error: Simple texture destroyed.");
	return m_texture->getInternalHandle();
}

	}
}
