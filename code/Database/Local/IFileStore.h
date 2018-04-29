/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_IFileStore_H
#define traktor_db_IFileStore_H

#include "Core/Object.h"
#include "Core/Io/Path.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_LOCAL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class ConnectionString;

/*! \brief File store interface.
 * \ingroup Database
 */
class T_DLLCLASS IFileStore : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool create(const ConnectionString& connectionString) = 0;

	virtual void destroy() = 0;

	virtual bool locked(const Path& filePath) = 0;

	virtual bool pending(const Path& filePath) = 0;

	virtual bool add(const Path& filePath) = 0;

	virtual bool remove(const Path& filePath) = 0;

	virtual bool edit(const Path& filePath) = 0;

	virtual bool rollback(const Path& filePath) = 0;

	virtual bool clean(const Path& filePath) = 0;

};

	}
}

#endif	// traktor_db_IFileStore_H
