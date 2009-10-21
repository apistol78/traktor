#ifndef traktor_editor_PipelineDependency_H
#define traktor_editor_PipelineDependency_H

#include <set>
#include "Core/Object.h"
#include "Core/Heap/Ref.h"
#include "Core/Io/Path.h"
#include "Core/Guid.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Serializable;

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
	T_RTTI_CLASS(PipelineDependency)

public:
	PipelineDependency();

	std::wstring name;
	Ref< IPipeline > pipeline;
	Ref< const Serializable > sourceAsset;
	std::wstring outputPath;
	Guid outputGuid;
	Ref< const Object > buildParams;
	std::set< Path > files;
	uint32_t checksum;
	bool build;
	uint32_t reason;
	Ref< PipelineDependency > parent;
	RefArray< PipelineDependency > children;
};

	}
}

#endif	// traktor_editor_PipelineDependency_H
