#pragma once

#include "Core/RefArray.h"
#include "Core/Io/Path.h"
#include "Editor/IPipelineCommon.h"
#include "Editor/PipelineTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::db
{

class Instance;

}

namespace traktor::editor
{

class IPipeline;

/*! Pipeline dependency interface.
 * \ingroup Editor
 */
class T_DLLCLASS IPipelineDepends : public IPipelineCommon
{
	T_RTTI_CLASS;

public:
	/*! Add dependency to source asset; will not produce any output. */
	virtual void addDependency(
		const ISerializable* sourceAsset
	) = 0;

	/*! Add dependency to source asset. */
	virtual void addDependency(
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		uint32_t flags
	) = 0;

	/*! Add dependency to source asset. */
	virtual void addDependency(
		db::Instance* sourceAssetInstance,
		uint32_t flags
	) = 0;

	/*! Add dependency to source asset. */
	virtual void addDependency(
		const Guid& sourceAssetGuid,
		uint32_t flags
	) = 0;

	/*! Add dependency to physical file. */
	virtual void addDependency(
		const Path& basePath,
		const std::wstring& fileName
	) = 0;

	/*! Add dependency to pipeline which consume source assets of specified type. */
	virtual void addDependency(
		const TypeInfo& sourceAssetType
	) = 0;

	/*! Wait until all dependency processing is complete before returning. */
	virtual bool waitUntilFinished() = 0;

	/*! Add dependency to pipeline which consume source assets of specified type. */
	template < typename T >
	void addDependency()
	{
		addDependency(type_of< T >());
	}
};

}
