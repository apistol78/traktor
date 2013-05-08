#ifndef traktor_db_MsgHandleArrayResult_H
#define traktor_db_MsgHandleArrayResult_H

#include <vector>
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

/*! \brief Handle array result.
 * \ingroup Database
 */
class T_DLLCLASS MsgHandleArrayResult : public IMessage
{
	T_RTTI_CLASS;

public:
	void add(uint32_t handle);

	uint32_t count();

	uint32_t get(uint32_t index) const;

	virtual void serialize(ISerializer& s);

private:
	std::vector< uint32_t > m_handles;
};

	}
}

#endif	// traktor_db_MsgHandleArrayResult_H