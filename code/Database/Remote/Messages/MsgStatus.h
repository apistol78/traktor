#ifndef traktor_db_MsgStatus_H
#define traktor_db_MsgStatus_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief
 */
enum MsgStatusType
{
	StSuccess = 0,
	StFailure = -1,
	StNoReply = -2,
	StInvalidReply = -3
};

/*! \brief
 */
class MsgStatus : public IMessage
{
	T_RTTI_CLASS(MsgStatus)

public:
	MsgStatus(MsgStatusType status = StSuccess);

	MsgStatusType getStatus() const { return (MsgStatusType)m_status; }

	virtual bool serialize(Serializer& s);

private:
	int32_t m_status;
};

	}
}

#endif	// traktor_db_MsgStatus_H
