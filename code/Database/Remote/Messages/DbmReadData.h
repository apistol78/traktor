#ifndef traktor_db_DbmReadData_H
#define traktor_db_DbmReadData_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief Read instance data.
 * \ingroup Database
 */
class DbmReadData : public IMessage
{
	T_RTTI_CLASS(DbmReadData)

public:
	DbmReadData(uint32_t handle = 0, const std::wstring& name = L"");

	uint32_t getHandle() const { return m_handle; }

	const std::wstring& getName() const { return m_name; }

	virtual bool serialize(Serializer& s);

private:
	uint32_t m_handle;
	std::wstring m_name;
};

	}
}

#endif	// traktor_db_DbmReadData_H
