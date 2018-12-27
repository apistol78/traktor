/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

	virtual bool execute(Context* context) override final;

	virtual bool undo(Context* context) override final;

	virtual void clean(Context* context) override final;

	virtual bool redundant(const Action* action) const override final;

private:
	Path m_instancePath;
	Guid m_newGuid;
	bool m_create;
	bool m_editMeta;
};

	}
}

#endif	// traktor_db_ActionSetGuid_H
