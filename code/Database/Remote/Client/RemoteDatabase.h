#ifndef traktor_db_RemoteDatabase_H
#define traktor_db_RemoteDatabase_H

#include "Database/Provider/IProviderDatabase.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_CLIENT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class RemoteConnection;

/*! \brief Remote database provider.
 * \ingroup Database
 */
class T_DLLCLASS RemoteDatabase : public IProviderDatabase
{
	T_RTTI_CLASS;

public:
	virtual bool create(const ConnectionString& connectionString) T_OVERRIDE T_FINAL;

	virtual bool open(const ConnectionString& connectionString) T_OVERRIDE T_FINAL;

	virtual void close() T_OVERRIDE T_FINAL;

	virtual Ref< IProviderBus > getBus() T_OVERRIDE T_FINAL;

	virtual Ref< IProviderGroup > getRootGroup() T_OVERRIDE T_FINAL;

private:
	Ref< RemoteConnection > m_connection;
};

	}
}

#endif	// traktor_db_RemoteDatabase_H
