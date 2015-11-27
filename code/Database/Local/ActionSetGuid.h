#ifndef traktor_db_ActionSetGuid_H
#define traktor_db_ActionSetGuid_H

#include "Database/Local/Action.h"
#include "Core/Io/Path.h"
#include "Core/Guid.h"

namespace traktor
{
	namespace db
	{

/*! \brief Transaction set guid action.
 * \ingroup Database
 */
class ActionSetGuid : public Action
{
	T_RTTI_CLASS;

public:
	ActionSetGuid(const Path& instancePath, const Guid& newGuid, bool create);

	virtual bool execute(Context* context) T_OVERRIDE T_FINAL;

	virtual bool undo(Context* context) T_OVERRIDE T_FINAL;

	virtual void clean(Context* context) T_OVERRIDE T_FINAL;

	virtual bool redundant(const Action* action) const T_OVERRIDE T_FINAL;

private:
	Path m_instancePath;
	Guid m_newGuid;
	bool m_create;
	bool m_editMeta;
};

	}
}

#endif	// traktor_db_ActionSetGuid_H
