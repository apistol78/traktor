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

	virtual bool open(const std::wstring& connectionString);

	virtual void close();

	virtual void beginTransaction();

	virtual void endTransaction();

	virtual void setDependency(const Guid& guid, const DependencyHash& hash);

	virtual bool getDependency(const Guid& guid, DependencyHash& outHash) const;

	virtual void setFile(const Path& path, const FileHash& file);

	virtual bool getFile(const Path& path, FileHash& outFile);

	virtual Ref< IPipelineReport > createReport(const std::wstring& name, const Guid& guid);

private:
	mutable ReaderWriterLock m_lock;
	mutable StringOutputStream m_ss;
	Ref< sql::IConnection > m_connection;
	bool m_transaction;
};

	}
}

#endif	// traktor_editor_PipelineDb_H
