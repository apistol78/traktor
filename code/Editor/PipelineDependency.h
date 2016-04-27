#ifndef traktor_editor_PipelineDependency_H
#define traktor_editor_PipelineDependency_H

#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallSet.h"
#include "Core/Date/DateTime.h"
#include "Core/Io/Path.h"
#include "Core/Serialization/ISerializable.h"
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
class T_DLLCLASS PipelineDependency : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct ExternalFile
	{
		Path filePath;
		DateTime lastWriteTime;

		void serialize(ISerializer& s);
	};

	const TypeInfo* pipelineType;
	Guid sourceInstanceGuid;
	Ref< const ISerializable > sourceAsset;		/*!< Source asset. */
	AlignedVector< ExternalFile > files;		/*!< External file dependencies. */
	std::wstring outputPath;					/*!< Database output path. */
	Guid outputGuid;							/*!< Database output guid. */
	uint32_t pipelineHash;						/*!< Hash of pipeline settings. */
	uint32_t sourceAssetHash;					/*!< Hash of source asset. */
	uint32_t sourceDataHash;					/*!< Hash of source instance data. */
	uint32_t filesHash;							/*!< Hash of external files. */
	uint32_t flags;								/*!< Dependency flags. \sa PipelineDependencyFlags */
	SmallSet< uint32_t > children;				/*!< Child dependencies. */

	PipelineDependency();

	void dump() const;

	virtual void serialize(ISerializer& s);
};

	}
}

#endif	// traktor_editor_PipelineDependency_H
