#ifndef traktor_editor_PipelineDependency_H
#define traktor_editor_PipelineDependency_H

#include <set>
#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Io/Path.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

	namespace editor
	{

class IPipeline;

/*! \brief Pipeline asset dependency.
 * \ingroup Editor
 *
 * Describe the dependencies of an asset;
 * which other assets are used, which files are used etc.
 */
class T_DLLCLASS PipelineDependency : public Object
{
	T_RTTI_CLASS;

public:
	PipelineDependency();

	std::wstring name;							/*!< Name of output dependency. */
	Ref< IPipeline > pipeline;					/*!< Associated pipeline, must be used to build output. */
	uint32_t pipelineHash;						/*!< Hash of pipeline settings. */
	Ref< const ISerializable > sourceAsset;		/*!< Source asset. */
	uint32_t sourceAssetHash;					/*!< Hash of source asset. */
	std::wstring outputPath;					/*!< Database output path. */
	Guid outputGuid;							/*!< Database output guid. */
	Ref< const Object > buildParams;			/*!< Build parameters, passed from dependency phase. */
	std::set< Path > files;						/*!< External file dependencies. */
	bool build;									/*!< If dependency is required to build. */
	uint32_t reason;							/*!< Build reason, updated prior to being built. */
	PipelineDependency* parent;					/*!< Parent dependency. */
	RefArray< PipelineDependency > children;	/*!< Child dependencies. */
};

	}
}

#endif	// traktor_editor_PipelineDependency_H
