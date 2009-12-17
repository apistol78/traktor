#ifndef traktor_editor_PipelineDb_H
#define traktor_editor_PipelineDb_H

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
	namespace sql
	{

class IConnection;

	}

	namespace editor
	{

/*! \brief Pipeline database.
 * \ingroup Editor
 *
 * The pipeline db keeps records of builds
 * used by the pipeline to check if an asset
 * needs to be rebuilt.
 */
class T_DLLCLASS PipelineDb : public Object
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

	bool open(const std::wstring& connectionString);

	void close();

	void set(const Guid& guid, const Hash& hash);

	bool get(const Guid& guid, Hash& outHash) const;

private:
	Ref< sql::IConnection > m_connection;
};

	}
}

#endif	// traktor_editor_PipelineDb_H
