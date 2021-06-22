#pragma once

#include "Core/Io/StreamStream.h"

namespace traktor
{
	namespace db
	{

class BlockFile;

class BlockReadStream : public StreamStream
{
public:
	BlockReadStream(BlockFile* blockFile, IStream* stream, int64_t endOffset);

	virtual ~BlockReadStream();

	virtual void close() override final;

private:
	Ref< BlockFile > m_blockFile;
};

	}
}

