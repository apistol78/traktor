#include "Core/Io/StreamCopy.h"
#include "Core/Io/Stream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.StreamCopy", StreamCopy, Object)

StreamCopy::StreamCopy(Stream* target, Stream* source)
:	m_target(target)
,	m_source(source)
{
}

bool StreamCopy::execute()
{
	if (!m_target || !m_target->canWrite())
		return false;
	if (!m_source || !m_source->canRead())
		return false;

	uint8_t buffer[4096];

	for (;;)
	{
		int32_t nread = m_source->read(buffer, sizeof(buffer));
		if (nread < 0)
			return false;

		if (nread == 0)
			break;

		int32_t nwritten = m_target->write(buffer, nread);
		if (nwritten != nread)
			return false;
	}

	return true;
}

}
