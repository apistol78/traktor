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

int32_t SimpleTextureCapture::getMips() const
{
	T_CAPTURE_ASSERT (m_texture, L"Simple texture destroyed.");
	return m_texture ? m_texture->getMips() : 0;
}

int32_t SimpleTextureCapture::getWidth() const
{
	T_CAPTURE_ASSERT (m_texture, L"Simple texture destroyed.");
	return m_texture ? m_texture->getWidth() : 0;
}

int32_t SimpleTextureCapture::getHeight() const
{
	T_CAPTURE_ASSERT (m_texture, L"Simple texture destroyed.");
	return m_texture ? m_texture->getHeight() : 0;
}

bool SimpleTextureCapture::lock(int32_t level, Lock& lock)
{
	T_CAPTURE_ASSERT (m_texture, L"Simple texture destroyed.");
	T_CAPTURE_ASSERT (level >= 0, L"Invalid mip level.");
	T_CAPTURE_ASSERT (m_locked < 0, L"Already locked.");
	return m_texture ? m_texture->lock(level, lock) : false;
}

void SimpleTextureCapture::unlock(int32_t level)
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
