#ifndef traktor_editor_PipelineInstanceCache_H
#define traktor_editor_PipelineInstanceCache_H

#include <map>
#include "Core/Thread/Semaphore.h"
#include "Editor/IPipelineInstanceCache.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Database;

	}

	namespace editor
	{

/*! \brief Pipeline database instance object read-only cache.
 * \ingroup Editor
 */
class T_DLLCLASS PipelineInstanceCache : public IPipelineInstanceCache
{
	T_RTTI_CLASS;

public:
	PipelineInstanceCache(db::Database* database, const std::wstring& cacheDirectory);

	virtual Ref< ISerializable > getObjectReadOnly(const Guid& instanceGuid);

	virtual void flush(const Guid& instanceGuid);

private:
	Semaphore m_lock;
	Ref< db::Database > m_database;
	std::wstring m_cacheDirectory;
	std::map< Guid, Ref< ISerializable > > m_readCache;
};

	}
}

#endif	// traktor_editor_PipelineInstanceCache_H
