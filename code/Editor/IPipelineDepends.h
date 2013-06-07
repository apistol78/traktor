#ifndef traktor_editor_IPipelineDepends_H
#define traktor_editor_IPipelineDepends_H

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

namespace traktor
{
	namespace db
	{

class Instance;

	}

	namespace editor
	{

class IPipeline;

/*! \brief Pipeline dependency interface.
 * \ingroup Editor
 */
class T_DLLCLASS IPipelineDepends : public IPipelineCommon
{
	T_RTTI_CLASS;

public:
	/*! \brief Add dependency to source asset; will not produce any output. */
	virtual void addDependency(
		const ISerializable* sourceAsset
	) = 0;

	/*! \brief Add dependency to source asset. */
	virtual void addDependency(
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		uint32_t flags
	) = 0;

	/*! \brief Add dependency to source asset. */
	virtual void addDependency(
		db::Instance* sourceAssetInstance,
		uint32_t flags
	) = 0;

	/*! \brief Add dependency to source asset. */
	virtual void addDependency(
		const Guid& sourceAssetGuid,
		uint32_t flags
	) = 0;

	/*! \brief Add dependency to physical file. */
	virtual void addDependency(
		const Path& basePath,
		const std::wstring& fileName
	) = 0;

	/*! \brief Add dependency to pipeline which consume source assets of specified type. */
	virtual void addDependency(
		const TypeInfo& sourceAssetType
	) = 0;

	/*! \brief Wait until all dependency processing is complete before returning. */
	virtual bool waitUntilFinished() = 0;

	template < typename T >
	void addDependency()
	{
		addDependency(type_of< T >());
	}
};

	}
}

#endif	// traktor_editor_IPipelineDepends_H
