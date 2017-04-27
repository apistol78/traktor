/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_DbmGetRootGroup_H
#define traktor_db_DbmGetRootGroup_H

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

/*! \brief Get database root group.
 * \ingroup Database
 */
class T_DLLCLASS DbmGetRootGroup : public IMessage
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_db_DbmGetRootGroup_H
