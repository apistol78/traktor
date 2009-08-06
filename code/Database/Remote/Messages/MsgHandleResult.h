#ifndef traktor_db_MsgHandleResult_H
#define traktor_db_MsgHandleResult_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief
 */
class MsgHandleResult : public IMessage
{
	T_RTTI_CLASS(MsgHandleResult)

public:
	MsgHandleResult(uint32_t handle = 0);

	uint32_t get() const { return m_handle; }

	virtual bool serialize(Serializer& s);

private:
	uint32_t m_handle;
};

	}
}

#endif	// traktor_db_MsgHandleResult_H