#include "Render/SequenceTexture.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SequenceTexture", SequenceTexture, ITexture)

void SequenceTexture::destroy()
{
	m_textures.clear();
}

ITexture* SequenceTexture::resolve()
{
	int32_t index = int32_t(m_rate * m_time.getElapsedTime()) % int32_t(m_textures.size());
	return m_textures[index];
}

	}
}
