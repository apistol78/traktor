#include "Core/Io/IStream.h"
#include "Model/Formats/LwChunk.h"
#include "Model/Formats/LwRead.h"
#include "Model/Formats/LwVMap.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.LwVMap", LwVMap, Object)

LwVMap::LwVMap()
:	m_type(0)
,	m_dimension(0)
{
}

bool LwVMap::read(const LwChunk& chunk, IStream* stream)
{
	float e[16];

	if (!lwRead< uint32_t >(stream, m_type))
		return false;

	if (!lwRead< uint16_t >(stream, m_dimension) || m_dimension > sizeof_array(e))
		return false;

	if (!lwRead< std::wstring >(stream, m_name))
		return false;

	while (stream->tell() < chunk.end)
	{
		uint32_t vertex;
		if (!lwReadIndex(stream, vertex))
			return false;

		if (!lwRead< float >(stream, e, m_dimension))
			return false;

		m_vm[vertex] = m_values.size();
		m_values.insert(m_values.end(), &e[0], &e[m_dimension]);
	}

	return true;
}

	}
}
