/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_IEnvironment_H
#define traktor_amalgam_IEnvironment_H

#include "Core/Object.h"
#include "Amalgam/Run/IScriptServer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_RUN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;

	namespace db
	{

class Database;

	}

	namespace amalgam
	{

/*! \brief Runtime environment.
 * \ingroup Amalgam
 *
 * This class represent the runtime environment.
 * It's a single accessors for applications to reach
 * various system objects.
 */
class T_DLLCLASS IEnvironment : public Object
{
	T_RTTI_CLASS;

public:
	virtual db::Database* getDatabase() = 0;

	virtual IScriptServer* getScript() = 0;

	virtual PropertyGroup* getSettings() = 0;

	virtual bool alive() const = 0;
};

	}
}

#endif	// traktor_amalgam_IEnvironment_H
