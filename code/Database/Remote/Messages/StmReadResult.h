#ifndef traktor_db_StmReadResult_H
#define traktor_db_StmReadResult_H

#include "Database/Remote/IMessage.h"
#include "Database/Remote/Messages/StmRead.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

/*! \brief Read block result.
 * \ingroup Database
 */
class T_DLLCLASS StmReadResult : public IMessage
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