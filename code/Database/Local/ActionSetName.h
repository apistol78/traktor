#ifndef traktor_db_ActionSetName_H
#define traktor_db_ActionSetName_H

#include "Database/Local/Action.h"
#include "Core/Io/Path.h"

namespace traktor
{
	namespace db
	{

/*! \brief Transaction set name action.
 * \ingroup Database
 */
class ActionSetName : public Action
{
	T_RTTI_CLASS;

public:
	ActionSetName(const Path& instancePath, const std::wstring& newName);

	virtual bool execute(Context* context);

	virtual bool undo(Context* context);

	virtual void clean(Context* context);

private:
	Path m_instancePath;
	std::wstring m_newName;
	bool m_renamedMeta;
	bool m_renamedObject;
};

	}
}

#endif	// traktor_db_ActionSetName_H
