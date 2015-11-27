#ifndef traktor_db_DbmReadObjectResult_H
#define traktor_db_DbmReadObjectResult_H

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

/*! \brief Read object result.
 * \ingroup Database
 */
class T_DLLCLASS DbmReadObjectResult : public IMessage
{
	T_RTTI_CLASS;

public:
	DbmReadObjectResult(uint32_t streamId = 0, const std::wstring& serializerTypeName = L"");

	uint32_t getStreamId() const { return m_streamId; }

	const std::wstring& getSerializerTypeName() const { return m_serializerTypeName; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	uint32_t m_streamId;
	std::wstring m_serializerTypeName;
};

	}
}

#endif	// traktor_db_DbmReadObjectResult_H