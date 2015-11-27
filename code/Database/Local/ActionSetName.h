#ifndef traktor_db_ActionSetName_H
#define traktor_db_ActionSetName_H

#include <map>
#include "Core/Io/Path.h"
#include "Database/Local/Action.h"

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

	virtual bool execute(Context* context) T_OVERRIDE T_FINAL;

	virtual bool undo(Context* context) T_OVERRIDE T_FINAL;

	virtual void clean(Context* context) T_OVERRIDE T_FINAL;

	virtual bool redundant(const Action* action) const T_OVERRIDE T_FINAL;

private:
	Path m_instancePath;
	Path m_instancePathNew;
	bool m_removedMeta;
	bool m_removedObject;
	std::map< std::wstring, bool > m_removedData;
};

	}
}

#endif	// traktor_db_ActionSetName_H
