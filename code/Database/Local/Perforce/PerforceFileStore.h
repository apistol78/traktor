/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_PerforceFileStore_H
#define traktor_db_PerforceFileStore_H

#include "Database/Local/IFileStore.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_LOCAL_P4_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class PerforceClient;
class PerforceChangeList;

/*! \brief Perforce file store.
 * \ingroup Database
 */
class T_DLLCLASS PerforceFileStore : public IFileStore
{
	T_RTTI_CLASS;

public:
	virtual bool create(const ConnectionString& connectionString) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual uint32_t flags(const Path& filePath) T_OVERRIDE T_FINAL;

	virtual bool add(const Path& filePath) T_OVERRIDE T_FINAL;

	virtual bool remove(const Path& filePath) T_OVERRIDE T_FINAL;

	virtual bool edit(const Path& filePath) T_OVERRIDE T_FINAL;

	virtual bool rollback(const Path& filePath) T_OVERRIDE T_FINAL;

	virtual bool clean(const Path& filePath) T_OVERRIDE T_FINAL;

private:
	Ref< PerforceClient > m_p4client;
	Ref< PerforceChangeList > m_p4changeList;
};

	}
}

#endif	// traktor_db_PerforceFileStore_H
