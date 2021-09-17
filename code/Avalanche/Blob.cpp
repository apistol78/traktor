#include "Avalanche/Blob.h"
#include "Core/Io/ChunkMemory.h"
#include "Core/Io/ChunkMemoryStream.h"

namespace traktor
{
	namespace avalanche
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.avalanche.Blob", Blob, Object)

Blob::Blob()
:	m_memory(new ChunkMemory())
{
}

int64_t Blob::size() const
{
	return m_memory->size();
}

Ref< IStream > Blob::append(int64_t appendSize)
{
	Ref< ChunkMemoryStream > stream = new ChunkMemoryStream(m_memory, false, true);
	stream->seek(IStream::SeekEnd, 0);
	return stream;
}

Ref< IStream > Blob::read() const
{
	return new ChunkMemoryStream(m_memory, true, false);
}

	}
}
