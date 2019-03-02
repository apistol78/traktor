#include <algorithm>
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Misc/AutoPtr.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.StreamCopy", StreamCopy, Object)

StreamCopy::StreamCopy(IStream* target, IStream* source)
:	m_target(target)
,	m_source(source)
{
}

bool StreamCopy::execute(uint64_t copyBytes)
{
	if (!m_target || !m_target->canWrite())
		return false;
	if (!m_source || !m_source->canRead())
		return false;
	if (copyBytes <= 0)
		return true;

	AutoArrayPtr< uint8_t > buffer(new uint8_t [65536]);

	while (copyBytes > 0)
	{
		uint64_t nget = std::min< uint64_t >(copyBytes, 65536);

		int64_t nread = m_source->read(buffer.ptr(), int64_t(nget));
		if (nread < 0)
			return false;

		if (nread == 0)
			break;

		int64_t nwritten = m_target->write(buffer.ptr(), nread);
		if (nwritten != nread)
			return false;

		copyBytes -= nget;
	}

	return true;
}

}
