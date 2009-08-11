#ifndef traktor_db_DbmReadObjectResult_H
#define traktor_db_DbmReadObjectResult_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief Read object result.
 * \ingroup Database
 */
class DbmReadObjectResult : public IMessage
{
	T_RTTI_CLASS(DbmReadObjectResult)

public:
	DbmReadObjectResult(uint32_t handle = 0, const std::wstring& serializerTypeName = L"");

	uint32_t getHandle() const { return m_handle; }

	const std::wstring& getSerializerTypeName() const { return m_serializerTypeName; }

	virtual bool serialize(Serializer& s);

private:
	uint32_t m_handle;
	std::wstring m_serializerTypeName;
};

	}
}

#endif	// traktor_db_DbmReadObjectResult_H