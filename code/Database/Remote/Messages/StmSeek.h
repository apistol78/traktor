#ifndef traktor_db_StmSeek_H
#define traktor_db_StmSeek_H

#include "Database/Remote/IMessage.h"
#include "Core/Io/IStream.h"

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

/*! \brief Seek stream.
 * \ingroup Database
 */
class T_DLLCLASS StmSeek : public IMessage
{
	T_RTTI_CLASS;

public:
	StmSeek(uint32_t handle = 0, IStream::SeekOriginType origin = IStream::SeekCurrent, int32_t offset = 0);

	uint32_t getHandle() const { return m_handle; }

	IStream::SeekOriginType getOrigin() const { return IStream::SeekOriginType(m_origin); }

	int32_t getOffset() const { return m_offset; }

	virtual bool serialize(ISerializer& s);

private:
	uint32_t m_handle;
	int32_t m_origin;
	int32_t m_offset;
};

	}
}

#endif	// traktor_db_StmSeek_H