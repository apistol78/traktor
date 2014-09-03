#ifndef traktor_editor_PipelineDbFlat_H
#define traktor_editor_PipelineDbFlat_H

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
	namespace editor
	{

class T_DLLCLASS PipelineDbFlat : public IPipelineDb
{
	T_RTTI_CLASS;

public:
	PipelineDbFlat();

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
	std::wstring m_file;
	std::map< Guid, DependencyHash > m_dependencies;
	std::map< std::wstring, FileHash > m_files;
	uint32_t m_changes;
};

	}
}

#endif	// traktor_editor_PipelineDbFlat_H
