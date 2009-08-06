#ifndef traktor_db_DbmWriteObjectResult_H
#define traktor_db_DbmWriteObjectResult_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief
 */
class DbmWriteObjectResult : public IMessage
{
	T_RTTI_CLASS(DbmWriteObjectResult)

public:
	DbmWriteObjectResult(uint32_t handle = 0, const std::wstring& serializerTypeName = L"");

	uint32_t getHandle() const { return m_handle; }

	const std::wstring& getSerializerTypeName() const { return m_serializerTypeName; }

	virtual bool serialize(Serializer& s);

private:
	uint32_t m_handle;
	std::wstring m_serializerTypeName;
};

	}
}

#endif	// traktor_db_DbmWriteObjectResult_H