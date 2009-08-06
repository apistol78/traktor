#ifndef traktor_db_StmWrite_H
#define traktor_db_StmWrite_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief
 */
class StmWrite : public IMessage
{
	T_RTTI_CLASS(StmWrite)

public:
	enum { MaxBlockSize = 2048 };

	StmWrite(uint32_t handle = 0, int32_t blockSize = 0, const void* block = 0);

	uint32_t getHandle() const { return m_handle; }

	int32_t getBlockSize() const { return m_blockSize; }

	const void* getBlock() const { return m_block; }

	virtual bool serialize(Serializer& s);

private:
	uint32_t m_handle;
	int32_t m_blockSize;
	uint8_t m_block[MaxBlockSize];
};

	}
}

#endif	// traktor_db_StmWrite_H