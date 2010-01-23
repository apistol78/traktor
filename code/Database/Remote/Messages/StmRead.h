#ifndef traktor_db_StmRead_H
#define traktor_db_StmRead_H

#include "Database/Remote/IMessage.h"

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

/*! \brief Read block from stream.
 * \ingroup Database
 */
class T_DLLCLASS StmRead : public IMessage
{
	T_RTTI_CLASS;

public:
	enum { MaxBlockSize = 2048 };

	StmRead(uint32_t handle = 0, int32_t blockSize = 0);

	uint32_t getHandle() const { return m_handle; }

	int32_t getBlockSize() const { return m_blockSize; }

	virtual bool serialize(ISerializer& s);

private:
	uint32_t m_handle;
	int32_t m_blockSize;
};

	}
}

#endif	// traktor_db_StmRead_H