#ifndef traktor_db_BlockFile_H
#define traktor_db_BlockFile_H

#include "Core/Object.h"
#include "Core/Io/Path.h"
#include "Core/Thread/Mutex.h"

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
		uint32_t offset;
		uint32_t size;
	};

	virtual ~BlockFile();

	bool create(const Path& fileName);

	bool open(const Path& fileName, bool readOnly);

	void close();

	uint32_t allocBlockId();

	void freeBlockId(uint32_t blockId);

	Ref< IStream > readBlock(uint32_t blockId);

	Ref< IStream > writeBlock(uint32_t blockId);

	void flushTOC();

private:
	Mutex m_lock;
	Ref< IStream > m_stream;
	std::vector< Block > m_blocks;
};

	}
}

#endif	// traktor_db_BlockFile_H
