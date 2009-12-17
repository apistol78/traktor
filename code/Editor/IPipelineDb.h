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
	struct Hash
	{
		uint32_t pipelineVersion;
		uint32_t pipelineHash;
		uint32_t sourceAssetHash;
		std::map< Path, DateTime > timeStamps;
	};

	virtual void set(const Guid& guid, const Hash& hash) = 0;

	virtual bool get(const Guid& guid, Hash& outHash) const = 0;

	virtual IPipelineReport* createReport(const std::wstring& name, const Guid& guid) = 0;
};

	}
}

#endif	// traktor_editor_IPipelineDb_H
