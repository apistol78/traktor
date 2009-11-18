#ifndef traktor_db_RemoteDatabase_H
#define traktor_db_RemoteDatabase_H

#include "Database/Provider/IProviderDatabase.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_CLIENT_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Connection;

/*! \brief Remote database provider.
 * \ingroup Database
 */
class T_DLLCLASS RemoteDatabase : public IProviderDatabase
{
	T_RTTI_CLASS;

public:
	bool open(const std::wstring& connectionString);

	virtual void close();

	virtual Ref< IProviderBus > getBus();

	virtual Ref< IProviderGroup > getRootGroup();

private:
	Ref< Connection > m_connection;
};

	}
}

#endif	// traktor_db_RemoteDatabase_H
