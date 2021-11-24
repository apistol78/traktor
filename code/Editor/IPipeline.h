#pragma once

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"
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

	namespace db
	{

class Instance;

	}

	namespace editor
	{

class IPipelineBuilder;
class IPipelineSettings;
class PipelineDependencySet;
class IPipelineDepends;
class PipelineDependency;

/*! Source asset pipeline.
 * \ingroup Editor
 */
class T_DLLCLASS IPipeline : public Object
{
	T_RTTI_CLASS;

public:
	/*! Create pipeline.
	 *
	 * \param settings Pipeline settings.
	 * \return True if pipeline created successfully.
	 */
	virtual bool create(const IPipelineSettings* settings) = 0;

	/*! Destroy pipeline. */
	virtual void destroy() = 0;

	/*! Get pipeline asset types. */
	virtual TypeInfoSet getAssetTypes() const = 0;

	/*! Return true if this pipeline benefit from being cached. */
	virtual bool shouldCache() const = 0;

	/*! Calculate hash of asset. */
	virtual uint32_t hashAsset(const ISerializable* sourceAsset) const = 0;

	/*! Build dependencies from source asset.
	 *
	 * \param pipelineDepends Pipeline dependency walker.
	 * \param sourceInstance Source asset database instance; might be null if asset doesn't originate from database.
	 * \param sourceAsset Source asset object.
	 * \param outputPath Output path in database.
	 * \param outputGuid Output instance guid.
	 * \param True if successful.
	 */
	virtual bool buildDependencies(
		IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid
	) const = 0;

	/*! Build output asset.
	 *
	 * \param pipelineBuilder Pipeline builder.
	 * \param dependencySet Dependency set.
	 * \param dependency Current dependency.
	 * \param sourceInstance Source asset database instance; null if asset doesn't originate from database.
	 * \param sourceAsset Source asset object.
	 * \param outputPath Output path in database.
	 * \param outputGuid Output instance guid.
	 * \param buildParams Optional parameters which is passed from another pipeline during build.
	 * \param reason Reason as why this asset needs to be rebuilt \sa PipelineBuildReason
	 * \return True if successful.
	 */
	virtual bool buildOutput(
		IPipelineBuilder* pipelineBuilder,
		const PipelineDependencySet* dependencySet,
		const PipelineDependency* dependency,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		const Object* buildParams,
		uint32_t reason
	) const = 0;

	/*! Build output product.
	 *
	 * Build output product from source asset.
	 *
	 * \note
	 * Implementations of this method should not generate any
	 * output instances, only return a product object.
	 *
	 * \param pipelineBuilder Pipeline builder.
	 * \param sourceInstance Source asset database instance; null if asset doesn't originate from database.
	 * \param sourceAsset Source asset object.
	 * \param buildParams Optional parameters which is passed from another pipeline during build.
	 * \return Output instance.
	 */
	virtual Ref< ISerializable > buildProduct(
		IPipelineBuilder* pipelineBuilder,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const Object* buildParams
	) const = 0;
};

	}
}

