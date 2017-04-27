/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_BlockFile_H
#define traktor_db_BlockFile_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Io/Path.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{

class IStream;

	namespace db
	{

/*! \brief Block file
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

	BlockFile();

	virtual ~BlockFile();

	bool create(const Path& fileName, bool flushAlways);

	bool open(const Path& fileName, bool readOnly, bool flushAlways);

	void close();

	uint32_t allocBlockId();

	void freeBlockId(uint32_t blockId);

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
	std::vector< Block > m_blocks;
	bool m_flushAlways;
	bool m_needFlushTOC;
};

	}
}

#endif	// traktor_db_BlockFile_H
