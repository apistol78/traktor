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
	virtual bool create(const ConnectionString& connectionString);

	virtual void destroy();

	virtual bool pending(const Path& filePath);

	virtual bool add(const Path& filePath);

	virtual bool remove(const Path& filePath);

	virtual bool edit(const Path& filePath);

	virtual bool rollback(const Path& filePath);

	virtual bool clean(const Path& filePath);

private:
	Ref< PerforceClient > m_p4client;
	Ref< PerforceChangeList > m_p4changeList;
};

	}
}

#endif	// traktor_db_PerforceFileStore_H
