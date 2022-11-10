/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Io/Path.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{

class IStream;

	namespace db
	{

/*! Block file
 * \ingroup Database
 */
class BlockFile : public Object
{
	T_RTTI_CLASS;

public:
	struct Block
	{
		uint32_t id;
		int64_t offset;
		int64_t size;
	};

	BlockFile() = default;

	virtual ~BlockFile();

	bool create(const Path& fileName, bool flushAlways);

	bool open(const Path& fileName, bool readOnly, bool flushAlways);

	void close();

	uint32_t allocBlockId();

	void freeBlockId(uint32_t blockId);

	int64_t allocateRegion(int64_t size);

	Ref< IStream > readBlock(uint32_t blockId);

	Ref< IStream > writeBlock(uint32_t blockId);

	void needFlushTOC();

	void flushTOC();

	void returnReadStream(IStream* readStream);

private:
	Path m_fileName;
	Semaphore m_lock;
	Ref< IStream > m_stream;
	RefArray< IStream > m_unusedReadStreams;
	AlignedVector< Block > m_blocks;
	bool m_flushAlways = false;
	bool m_needFlushTOC = false;
};

	}
}

