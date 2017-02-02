#ifndef traktor_db_MsgGetChildrenResult_H
#define traktor_db_MsgGetChildrenResult_H

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
class T_DLLCLASS MsgGetChildrenResult : public IMessage
{
	T_RTTI_CLASS;

public:
	void addGroup(uint32_t handle);

	void addInstance(uint32_t handle);

	const std::vector< uint32_t >& getGroups() const;

	const std::vector< uint32_t >& getInstances() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::vector< uint32_t > m_groupHandles;
	std::vector< uint32_t > m_instanceHandles;
};

	}
}

#endif	// traktor_db_MsgGetChildrenResult_H