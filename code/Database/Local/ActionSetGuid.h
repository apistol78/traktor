#pragma once

#include "Database/Local/Action.h"
#include "Core/Io/Path.h"
#include "Core/Guid.h"

namespace traktor
{
	namespace db
	{

/*! Transaction set guid action.
 * \ingroup Database
 */
class ActionSetGuid : public Action
{
	T_RTTI_CLASS;

public:
	explicit ActionSetGuid(const Path& instancePath, const Guid& newGuid, bool create);

	virtual bool execute(Context& context) override final;

	virtual bool undo(Context& context) override final;

	virtual void clean(Context& context) override final;

	virtual bool redundant(const Action* action) const override final;

private:
	Path m_instancePath;
	Guid m_newGuid;
	bool m_create;
	bool m_editMeta;
};

	}
}

