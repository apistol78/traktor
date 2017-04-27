/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_ActionRemove_H
#define traktor_db_ActionRemove_H

#include "Database/Local/Action.h"
#include "Core/Io/Path.h"

namespace traktor
{
	namespace db
	{

/*! \brief Transaction remove action.
 * \ingroup Database
 */
class ActionRemove : public Action
{
	T_RTTI_CLASS;

public:
	ActionRemove(const Path& instancePath);

	virtual bool execute(Context* context) T_OVERRIDE T_FINAL;

	virtual bool undo(Context* context) T_OVERRIDE T_FINAL;

	virtual void clean(Context* context) T_OVERRIDE T_FINAL;

	virtual bool redundant(const Action* action) const T_OVERRIDE T_FINAL;

private:
	Path m_instancePath;
	std::vector< std::wstring > m_renamedFiles;
};

	}
}

#endif	// traktor_db_ActionRemove_H
