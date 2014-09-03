#ifndef traktor_editor_IPipelineDb_H
#define traktor_editor_IPipelineDb_H

#include <map>
#include "Core/Guid.h"
#include "Core/Date/DateTime.h"
#include "Core/Io/Path.h"

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

class IPipelineReport;

/*! \brief Pipeline database.
 * \ingroup Editor
 *
 * The pipeline db keeps records of builds
 * used by the pipeline to check if an asset
 * needs to be rebuilt.
 *
 * It also keeps information about
 * the current status of the products
 * which can be aggregated to produce
 * statistics of the data.
 */
class T_DLLCLASS IPipelineDb : public Object
{
	T_RTTI_CLASS;

public:
	struct DependencyHash
	{
		uint32_t pipelineVersion;
		uint32_t hash;
	};

	struct FileHash
	{
		uint64_t size;
		DateTime lastWriteTime;
		uint32_t hash;
	};

	virtual bool open(const std::wstring& connectionString) = 0;

	virtual void close() = 0;

	virtual void beginTransaction() = 0;

	virtual void endTransaction() = 0;

	virtual void setDependency(const Guid& guid, const DependencyHash& hash) = 0;

	virtual bool getDependency(const Guid& guid, DependencyHash& outHash) const = 0;

	virtual void setFile(const Path& path, const FileHash& file) = 0;

	virtual bool getFile(const Path& path, FileHash& outFile) = 0;

	virtual Ref< IPipelineReport > createReport(const std::wstring& name, const Guid& guid) = 0;
};

	}
}

#endif	// traktor_editor_IPipelineDb_H
