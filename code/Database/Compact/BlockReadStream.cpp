/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Compact/BlockFile.h"
#include "Database/Compact/BlockReadStream.h"

namespace traktor::db
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
