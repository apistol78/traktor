#pragma once

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

	virtual bool open(const std::wstring& connectionString) override final;

	virtual void close() override final;

	virtual void beginTransaction() override final;

	virtual void endTransaction() override final;

	virtual void setDependency(const Guid& guid, const PipelineDependencyHash& hash) override final;

	virtual bool getDependency(const Guid& guid, PipelineDependencyHash& outHash) const override final;

	virtual void setFile(const Path& path, const PipelineFileHash& file) override final;

	virtual bool getFile(const Path& path, PipelineFileHash& outFile) override final;

private:
	mutable ReaderWriterLock m_lock;
	std::wstring m_file;
	std::map< Guid, PipelineDependencyHash > m_dependencies;
	std::map< std::wstring, PipelineFileHash > m_files;
	uint32_t m_changes;
};

	}
}

