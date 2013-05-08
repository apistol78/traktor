#ifndef traktor_db_MsgStatus_H
#define traktor_db_MsgStatus_H

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

/*! \brief Status codes.
 * \ingroup Database
 */
enum MsgStatusType
{
	StSuccess = 0,
	StFailure = -1,
	StNoReply = -2,
	StInvalidReply = -3
};

/*! \brief Status result.
 * \ingroup Database
 */
class T_DLLCLASS MsgStatus : public IMessage
{
	T_RTTI_CLASS;

public:
	MsgStatus(MsgStatusType status = StSuccess);

	MsgStatusType getStatus() const { return (MsgStatusType)m_status; }

	virtual void serialize(ISerializer& s);

private:
	int32_t m_status;
};

	}
}

#endif	// traktor_db_MsgStatus_H
