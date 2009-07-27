#ifndef traktor_db_ActionWriteObject_H
#define traktor_db_ActionWriteObject_H

#include "Database/Local/Action.h"
#include "Core/Heap/Ref.h"
#include "Core/Io/Path.h"

namespace traktor
{

class Serializable;

	namespace db
	{

/*! \brief Transaction write object action.
 * \ingroup Database
 */
class ActionWriteObject : public Action
{
	T_RTTI_CLASS(Action)

public:
	ActionWriteObject(const Path& instancePath, const Serializable* object);

	virtual bool execute(Context* context);

	virtual bool undo(Context* context);

	virtual void clean(Context* context);

private:
	Path m_instancePath;
	Ref< const Serializable > m_object;
	bool m_oldObjectRenamed;
	bool m_oldMetaRenamed;
};

	}
}

#endif	// traktor_db_ActionWriteObject_H
