#ifndef traktor_db_StmReadResult_H
#define traktor_db_StmReadResult_H

#include "Database/Remote/IMessage.h"
#include "Database/Remote/Messages/StmRead.h"

namespace traktor
{
	namespace db
	{

/*! \brief Read block result.
 * \ingroup Database
 */
class StmReadResult : public IMessage
{
	T_RTTI_CLASS;

public:
	StmReadResult(const void* block = 0, int32_t blockSize = 0);

	const void* getBlock() const { return m_block; }

	int32_t getBlockSize() const { return m_blockSize; }

	virtual bool serialize(ISerializer& s);

private:
	uint8_t m_block[StmRead::MaxBlockSize];
	int32_t m_blockSize;
};

	}
}

#endif	// traktor_db_StmReadResult_H