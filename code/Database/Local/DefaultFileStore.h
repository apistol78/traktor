#ifndef traktor_db_DefaultFileStore_H
#define traktor_db_DefaultFileStore_H

#include "Database/Local/IFileStore.h"

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

/*! \brief Default file store.
 * \ingroup Database
 */
class T_DLLCLASS DefaultFileStore : public IFileStore
{
	T_RTTI_CLASS;

public:
	virtual bool create(const ConnectionString& connectionString);

	virtual void destroy();

	virtual bool pending(const Path& filePath);

	virtual bool add(const Path& filePath);

	virtual bool remove(const Path& filePath);

	virtual bool edit(const Path& filePath);

	virtual bool rollback(const Path& filePath);

	virtual bool clean(const Path& filePath);
};

	}
}

#endif	// traktor_db_DefaultFileStore_H
