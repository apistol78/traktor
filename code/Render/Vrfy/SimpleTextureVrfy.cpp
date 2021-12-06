#include "Core/Misc/SafeDestroy.h"
#include "Render/Vrfy/Error.h"
#include "Render/Vrfy/ResourceTracker.h"
#include "Render/Vrfy/SimpleTextureVrfy.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SimpleTextureVrfy", SimpleTextureVrfy, ISimpleTexture)

SimpleTextureVrfy::SimpleTextureVrfy(ResourceTracker* resourceTracker, ISimpleTexture* texture)
:	m_resourceTracker(resourceTracker)
,	m_texture(texture)
,	m_locked(-1)
{
	m_resourceTracker->add(this);
}

SimpleTextureVrfy::~SimpleTextureVrfy()
{
	m_resourceTracker->remove(this);
}

void SimpleTextureVrfy::destroy()
{
	T_CAPTURE_ASSERT (m_texture, L"Simple texture already destroyed.");
	safeDestroy(m_texture);
}

ITexture* SimpleTextureVrfy::resolve()
{
	T_CAPTURE_ASSERT (m_texture, L"Simple texture destroyed.");
	return this;
}

int32_t SimpleTextureVrfy::getWidth() const
{
	T_CAPTURE_ASSERT (m_texture, L"Simple texture destroyed.");
	return m_texture ? m_texture->getWidth() : 0;
}

int32_t SimpleTextureVrfy::getHeight() const
{
	T_CAPTURE_ASSERT (m_texture, L"Simple texture destroyed.");
	return m_texture ? m_texture->getHeight() : 0;
}

int32_t SimpleTextureVrfy::getMips() const
{
	T_CAPTURE_ASSERT (m_texture, L"Simple texture destroyed.");
	return m_texture ? m_texture->getMips() : 0;
}

bool SimpleTextureVrfy::lock(int32_t level, Lock& lock)
{
	T_CAPTURE_ASSERT (m_texture, L"Simple texture destroyed.");
	T_CAPTURE_ASSERT (level >= 0, L"Invalid mip level.");
	T_CAPTURE_ASSERT (level < getMips(), L"Invalid mip level.");
	T_CAPTURE_ASSERT (m_locked < 0, L"Already locked.");
	return m_texture ? m_texture->lock(level, lock) : false;
}

void SimpleTextureVrfy::unlock(int32_t level)
{
	T_CAPTURE_ASSERT (m_texture, L"Simple texture destroyed.");
	T_CAPTURE_ASSERT (level >= 0, L"Invalid mip level.");
	T_CAPTURE_ASSERT (level < getMips(), L"Invalid mip level.");
	T_CAPTURE_ASSERT (m_locked != level, L"Trying to unlock incorrect mip level.");
	if (m_texture)
		m_texture->unlock(level);
	m_locked = -1;
}

void* SimpleTextureVrfy::getInternalHandle()
{
	T_CAPTURE_ASSERT (m_texture, L"Simple texture destroyed.");
	return m_texture ? m_texture->getInternalHandle() : nullptr;
}

	}
}
