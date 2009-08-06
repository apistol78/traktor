#ifndef traktor_db_StmGetStat_H
#define traktor_db_StmGetStat_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief
 */
class StmGetStat : public IMessage
{
	T_RTTI_CLASS(StmGetStat)

public:
	StmGetStat(uint32_t handle = 0);

	uint32_t getHandle() const { return m_handle; }

	virtual bool serialize(Serializer& s);

private:
	uint32_t m_handle;
};

	}
}

#endif	// traktor_db_StmGetStat_H