#ifndef traktor_db_MsgStringResult_H
#define traktor_db_MsgStringResult_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief Single string result.
 * \ingroup Database
 */
class MsgStringResult : public IMessage
{
	T_RTTI_CLASS(MsgStringResult)

public:
	MsgStringResult(const std::wstring& value = L"");

	const std::wstring& get() const { return m_value; }

	virtual bool serialize(Serializer& s);

private:
	std::wstring m_value;
};

	}
}

#endif	// traktor_db_MsgStringResult_H