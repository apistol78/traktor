#ifndef traktor_db_ActionWriteObject_H
#define traktor_db_ActionWriteObject_H

#include "Database/Local/Action.h"
#include "Core/Io/Path.h"

namespace traktor
{

class DynamicMemoryStream;

	namespace db
	{

/*! \brief Transaction write object action.
 * \ingroup Database
 */
class ActionWriteObject : public Action
{
	T_RTTI_CLASS;

public:
	ActionWriteObject(const Path& instancePath, const std::wstring& primaryTypeName, DynamicMemoryStream* stream);

	virtual bool execute(Context* context);

	virtual bool undo(Context* context);

	virtual void clean(Context* context);

private:
	Path m_instancePath;
	std::wstring m_primaryTypeName;
	Ref< DynamicMemoryStream > m_objectStream;
	bool m_oldObjectRenamed;
	bool m_oldMetaRenamed;
};

	}
}

#endif	// traktor_db_ActionWriteObject_H
