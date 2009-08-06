#ifndef traktor_db_StmSeek_H
#define traktor_db_StmSeek_H

#include "Database/Remote/IMessage.h"
#include "Core/Io/Stream.h"

namespace traktor
{
	namespace db
	{

/*! \brief
 */
class StmSeek : public IMessage
{
	T_RTTI_CLASS(StmSeek)

public:
	StmSeek(uint32_t handle = 0, Stream::SeekOriginType origin = Stream::SeekCurrent, int32_t offset = 0);

	uint32_t getHandle() const { return m_handle; }

	Stream::SeekOriginType getOrigin() const { return Stream::SeekOriginType(m_origin); }

	int32_t getOffset() const { return m_offset; }

	virtual bool serialize(Serializer& s);

private:
	uint32_t m_handle;
	int32_t m_origin;
	int32_t m_offset;
};

	}
}

#endif	// traktor_db_StmSeek_H