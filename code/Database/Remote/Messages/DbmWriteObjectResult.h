#ifndef traktor_db_DbmWriteObjectResult_H
#define traktor_db_DbmWriteObjectResult_H

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

/*! \brief Write instance object result.
 * \ingroup Database
 */
class T_DLLCLASS DbmWriteObjectResult : public IMessage
{
	T_RTTI_CLASS;

public:
	DbmWriteObjectResult(uint32_t handle = 0, const std::wstring& serializerTypeName = L"");

	uint32_t getHandle() const { return m_handle; }

	const std::wstring& getSerializerTypeName() const { return m_serializerTypeName; }

	virtual bool serialize(ISerializer& s);

private:
	uint32_t m_handle;
	std::wstring m_serializerTypeName;
};

	}
}

#endif	// traktor_db_DbmWriteObjectResult_H