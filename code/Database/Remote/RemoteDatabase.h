#ifndef traktor_db_CompactDatabase_H
#define traktor_db_CompactDatabase_H

#include "Database/Provider/IProviderDatabase.h"
#include "Net/Url.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

/*! \brief Remote database provider
 * \ingroup Database
 */
class T_DLLCLASS RemoteDatabase : public IProviderDatabase
{
	T_RTTI_CLASS(CompactDatabase)

public:
	bool open(const net::Url& url);

	virtual void close();

	virtual IProviderBus* getBus();

	virtual IProviderGroup* getRootGroup();
};

	}
}

#endif	// traktor_db_CompactDatabase_H
