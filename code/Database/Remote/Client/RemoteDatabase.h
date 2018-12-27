/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	virtual bool create(const ConnectionString& connectionString) override final;

	virtual bool open(const ConnectionString& connectionString) override final;

	virtual void close() override final;

	virtual Ref< IProviderBus > getBus() override final;

	virtual Ref< IProviderGroup > getRootGroup() override final;

private:
	Ref< RemoteConnection > m_connection;
};

	}
}

#endif	// traktor_db_RemoteDatabase_H
