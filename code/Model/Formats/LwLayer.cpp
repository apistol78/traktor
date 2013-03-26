#include "Model/Formats/LwChunk.h"
#include "Model/Formats/LwLayer.h"
#include "Model/Formats/LwRead.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.LwLayer", LwLayer, Object)

LwLayer::LwLayer()
:	m_index(0)
,	m_flags(0)
,	m_parent(0)
{
}

bool LwLayer::read(const LwChunk& chunk, IStream* stream)
{
	if (!lwRead< uint16_t >(stream, m_index))
		return false;
	
	if (!lwRead< uint16_t >(stream, m_flags))
		return false;

	if (!lwRead< Vector4 >(stream, m_pivot))
		return false;

	if (!lwRead< std::wstring >(stream, m_name))
		return false;

	if (stream->tell() < chunk.end - 2)
	{
		if (!lwRead< uint16_t >(stream, m_parent))
			return false;
	}

	return true;
}

	}
}
