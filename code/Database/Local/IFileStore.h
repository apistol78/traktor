#pragma once

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

/*! File store interface.
 * \ingroup Database
 *
 * A file store is an abstraction for file systems which
 * require special attention when modifying files, such
 * as a SCM like Perforce.
 * 
 * This enables automatic checkout of instances when
 * working from the editor.
 */
class T_DLLCLASS IFileStore : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool create(const ConnectionString& connectionString) = 0;

	virtual void destroy() = 0;

	virtual uint32_t flags(const Path& filePath) = 0;

	virtual bool add(const Path& filePath) = 0;

	virtual bool remove(const Path& filePath) = 0;

	virtual bool edit(const Path& filePath) = 0;

	virtual bool rollback(const Path& filePath) = 0;

	virtual bool clean(const Path& filePath) = 0;

};

	}
}

