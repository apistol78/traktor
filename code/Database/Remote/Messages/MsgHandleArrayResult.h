#ifndef traktor_db_MsgHandleArrayResult_H
#define traktor_db_MsgHandleArrayResult_H

#include <vector>
#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief
 */
class MsgHandleArrayResult : public IMessage
{
	T_RTTI_CLASS(MsgHandleArrayResult)

public:
	void add(uint32_t handle);

	uint32_t count();

	uint32_t get(uint32_t index) const;

	virtual bool serialize(Serializer& s);

private:
	std::vector< uint32_t > m_handles;
};

	}
}

#endif	// traktor_db_MsgHandleArrayResult_H