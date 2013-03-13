#ifndef traktor_db_ActionWriteObject_H
#define traktor_db_ActionWriteObject_H

#include "Core/Io/Path.h"
#include "Database/Local/Action.h"

namespace traktor
{

class IStream;

	namespace db
	{

/*! \brief Transaction write object action.
 * \ingroup Database
 */
class ActionWriteObject : public Action
{
	T_RTTI_CLASS;

public:
	ActionWriteObject(const Path& instancePath, const std::wstring& primaryTypeName);

	virtual bool execute(Context* context);

	virtual bool undo(Context* context);

	virtual void clean(Context* context);

	const Ref< IStream >& getStream() const { return m_objectStream; }

private:
	Path m_instancePath;
	std::wstring m_primaryTypeName;
	std::vector< uint8_t > m_objectBuffer;
	Ref< IStream > m_objectStream;
	bool m_editObject;
	bool m_editMeta;
};

	}
}

#endif	// traktor_db_ActionWriteObject_H
