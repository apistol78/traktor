/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_CompactDatabase_H
#define traktor_db_CompactDatabase_H

#include "Database/Provider/IProviderDatabase.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_COMPACT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Path;

	namespace db
	{

class CompactContext;
class CompactGroup;

/*! \brief Compact database provider
 * \ingroup Database
 */
class T_DLLCLASS CompactDatabase : public IProviderDatabase
{
	T_RTTI_CLASS;

public:
	CompactDatabase();

	virtual bool create(const ConnectionString& connectionString) T_OVERRIDE T_FINAL;

	virtual bool open(const ConnectionString& connectionString) T_OVERRIDE T_FINAL;

	virtual void close() T_OVERRIDE T_FINAL;

	virtual Ref< IProviderBus > getBus() T_OVERRIDE T_FINAL;

	virtual Ref< IProviderGroup > getRootGroup() T_OVERRIDE T_FINAL;

private:
	Ref< CompactContext > m_context;
	Ref< CompactGroup > m_rootGroup;
	bool m_readOnly;
	uint32_t m_registryHash;
};

	}
}

#endif	// traktor_db_CompactDatabase_H
