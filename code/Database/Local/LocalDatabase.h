#pragma once

#include "Database/Provider/IProviderDatabase.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_LOCAL_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Path;

	namespace db
	{

class Context;
class LocalBus;
class LocalGroup;

/*! Local database provider.
 * \ingroup Database
 */
class T_DLLCLASS LocalDatabase : public IProviderDatabase
{
	T_RTTI_CLASS;

public:
	virtual bool create(const ConnectionString& connectionString) override final;

	virtual bool open(const ConnectionString& connectionString) override final;

	virtual void close() override final;

	virtual Ref< IProviderBus > getBus() override final;

	virtual Ref< IProviderGroup > getRootGroup() override final;

private:
	Ref< Context > m_context;
	Ref< LocalBus > m_bus;
	Ref< LocalGroup > m_rootGroup;
};

	}
}

