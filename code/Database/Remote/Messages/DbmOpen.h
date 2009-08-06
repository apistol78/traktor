#ifndef traktor_db_DbmOpen_H
#define traktor_db_DbmOpen_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief
 */
class DbmOpen : public IMessage
{
	T_RTTI_CLASS(DbmOpen)

public:
	DbmOpen(const std::wstring& name = L"");

	const std::wstring& getName() const { return m_name; }

	virtual bool serialize(Serializer& s);

private:
	std::wstring m_name;
};

	}
}

#endif	// traktor_db_DbmOpen_H
