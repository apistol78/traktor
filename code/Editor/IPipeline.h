#ifndef traktor_editor_IPipeline_H
#define traktor_editor_IPipeline_H

#include "Core/Object.h"
#include "Core/Guid.h"
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

class IPipelineSettings;
class IPipelineDepends;
class IPipelineBuilder;

/*! \brief Source asset pipeline.
 * \ingroup Editor
 */
class T_DLLCLASS IPipeline : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Create pipeline.
	 *
	 * \param settings Pipeline settings.
	 * \return True if pipeline created successfully.
	 */
	virtual bool create(const IPipelineSettings* settings) = 0;

	/*! \brief Destroy pipeline. */
	virtual void destroy() = 0;

	/*! \brief Get pipeline asset types. */
	virtual TypeInfoSet getAssetTypes() const = 0;

	/*! \brief Build dependencies from source asset.
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

	/*! \brief Build output asset.
	 *
	 * \param pipelineBuilder Pipeline builder.
	 * \param sourceInstance Source asset database instance; might be null if asset doesn't originate from database.
	 * \param sourceAsset Source asset object.
	 * \param sourceAssetHash Source asset hash key.
	 * \param outputPath Output path in database.
	 * \param outputGuid Output instance guid.
	 * \param buildParams Parameters which is passed from another pipeline during build.
	 * \param reason Reason as why this asset needs to be rebuilt \sa PipelineBuildReason
	 * \return True if successful.
	 */
	virtual bool buildOutput(
		IPipelineBuilder* pipelineBuilder,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		uint32_t sourceAssetHash,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		const Object* buildParams,
		uint32_t reason
	) const = 0;

	/*! \brief Build output instance.
	 *
	 * Build output instance from source asset.
	 *
	 * \param pipelineBuilder Pipeline builder.
	 * \param sourceAsset Source asset object.
	 * \return Output instance.
	 */
	virtual Ref< ISerializable > buildOutput(
		IPipelineBuilder* pipelineBuilder,
		const ISerializable* sourceAsset
	) const = 0;
};

	}
}

#endif	// traktor_editor_IPipeline_H
