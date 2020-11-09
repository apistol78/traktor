#pragma once

#include "Core/RefArray.h"
#include "Editor/IPipelineCommon.h"

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

class IPipelineDependencySet;
class PipelineDependency;

/*! Pipeline builder interface.
 * \ingroup Editor
 */
class T_DLLCLASS IPipelineBuilder : public IPipelineCommon
{
	T_RTTI_CLASS;

public:
	enum BuildResult
	{
		BrSucceeded = 0,
		BrSucceededWithWarnings = 1,
		BrFailed = 2
	};

	struct IListener
	{
		virtual ~IListener() {}

		virtual void beginBuild(
			int32_t index,
			int32_t count,
			const PipelineDependency* dependency
		) = 0;

		virtual void endBuild(
			int32_t index,
			int32_t count,
			const PipelineDependency* dependency,
			BuildResult result
		) = 0;
	};

	//@name External interface, should be called from pipeline hosts.
	//@{

	/*! Build all instances in dependency set. */
	virtual bool build(const IPipelineDependencySet* dependencySet, bool rebuild) = 0;

	//@}

	//@name Build interface, should only be called from other pipelines during build.
	//@{

	/*! Build output instance (product) from source asset.
	 *
	 * \param sourceInstance Database instance which contain sourceAsset, passed to IPipeline::buildOutput directly.
	 * \param sourceAsset Source asset.
	 * \param buildParams Optional build parameters.
	 * \return Output instance.
	 */
	virtual Ref< ISerializable > buildOutput(const db::Instance* sourceInstance, const ISerializable* sourceAsset, const Object* buildParams = nullptr) = 0;

	/*! Build ad-hoc output instance from source asset. */
	virtual bool buildAdHocOutput(const ISerializable* sourceAsset, const std::wstring& outputPath, const Guid& outputGuid, const Object* buildParams = nullptr) = 0;

	/*! Get guid prepared for synthesized outputs. */
	virtual Guid synthesizeOutputGuid(uint32_t iterations) = 0;

	/*! Get build product of source asset. */
	virtual Ref< ISerializable > getBuildProduct(const ISerializable* sourceAsset) = 0;

	/*! Create output database instance. */
	virtual Ref< db::Instance > createOutputInstance(const std::wstring& instancePath, const Guid& instanceGuid) = 0;

	/*! Get reference to output database. */
	virtual Ref< db::Database > getOutputDatabase() const = 0;

	//@}
};

	}
}

