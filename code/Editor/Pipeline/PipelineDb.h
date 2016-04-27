#ifndef traktor_editor_PipelineDb_H
#define traktor_editor_PipelineDb_H

#include "Core/Io/StringOutputStream.h"
#include "Core/Thread/ReaderWriterLock.h"
#include "Editor/IPipelineDb.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sql
	{

class IConnection;

	}

	namespace editor
	{

class T_DLLCLASS PipelineDb : public IPipelineDb
{
	T_RTTI_CLASS;

public:
	PipelineDb();

	virtual bool open(const std::wstring& connectionString) T_OVERRIDE T_FINAL;

	virtual void close() T_OVERRIDE T_FINAL;

	virtual void beginTransaction() T_OVERRIDE T_FINAL;

	virtual void endTransaction() T_OVERRIDE T_FINAL;

	virtual void setDependency(const Guid& guid, const PipelineDependencyHash& hash) T_OVERRIDE T_FINAL;

	virtual bool getDependency(const Guid& guid, PipelineDependencyHash& outHash) const T_OVERRIDE T_FINAL;

	virtual void setFile(const Path& path, const PipelineFileHash& file) T_OVERRIDE T_FINAL;

	virtual bool getFile(const Path& path, PipelineFileHash& outFile) T_OVERRIDE T_FINAL;

	virtual Ref< IPipelineReport > createReport(const std::wstring& name, const Guid& guid) T_OVERRIDE T_FINAL;

private:
	mutable ReaderWriterLock m_lock;
	mutable StringOutputStream m_ss;
	Ref< sql::IConnection > m_connection;
	bool m_transaction;
};

	}
}

#endif	// traktor_editor_PipelineDb_H
