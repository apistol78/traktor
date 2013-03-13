#ifndef traktor_editor_PipelineDependency_H
#define traktor_editor_PipelineDependency_H

#include <set>
#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Io/Path.h"
#include "Editor/PipelineTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
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

	Ref< IPipeline > pipeline;					/*!< Associated pipeline, must be used to build output. */
	uint32_t pipelineHash;						/*!< Hash of pipeline settings. */
	Ref< const ISerializable > sourceAsset;		/*!< Source asset. */
	uint32_t sourceDataHash;					/*!< Hash of source instance data. */
	uint32_t sourceAssetHash;					/*!< Hash of source asset. */
	uint32_t dependencyHash;					/*!< Dependency hash, a complete hash of this dep and the children. */
	std::wstring outputPath;					/*!< Database output path. */
	Guid outputGuid;							/*!< Database output guid. */
	std::set< Path > files;						/*!< External file dependencies. */
	uint32_t flags;								/*!< Dependency flags. \sa PipelineDependencyFlags */
	uint32_t reason;							/*!< Build reason, updated prior to being built. \sa PipelineBuildReason */
	RefArray< PipelineDependency > children;	/*!< Child dependencies. */
};

	}
}

#endif	// traktor_editor_PipelineDependency_H
