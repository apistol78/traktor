#include "Database/Compact/BlockFile.h"
#include "Database/Compact/BlockReadStream.h"

namespace traktor
{
	namespace db
	{

BlockReadStream::BlockReadStream(BlockFile* blockFile, IStream* stream, int64_t endOffset)
:	StreamStream(stream, endOffset)
,	m_blockFile(blockFile)
{
}

BlockReadStream::~BlockReadStream()
{
	close();
}

void BlockReadStream::close()
{
	if (m_stream)
	{
		m_blockFile->returnReadStream(m_stream);
		m_stream = nullptr;
	}
}

	}
}
