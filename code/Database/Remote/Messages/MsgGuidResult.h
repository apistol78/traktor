#ifndef traktor_db_MsgGuidResult_H
#define traktor_db_MsgGuidResult_H

#include "Database/Remote/IMessage.h"
#include "Core/Guid.h"

namespace traktor
{
	namespace db
	{

/*! \brief Guid result.
 * \ingroup Database
 */
class MsgGuidResult : public IMessage
{
	T_RTTI_CLASS;

public:
	MsgGuidResult(const Guid& value = Guid());

	const Guid& get() const { return m_value; }

	virtual bool serialize(ISerializer& s);

private:
	Guid m_value;
};

	}
}

#endif	// traktor_db_MsgGuidResult_H