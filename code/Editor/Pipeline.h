#ifndef traktor_editor_Pipeline_H
#define traktor_editor_Pipeline_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
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

	namespace db
	{

class Database;
class Instance;

	}

	namespace editor
	{

class Settings;
class PipelineManager;

/*! \brief Source asset pipeline.
 * \ingroup Editor
 */
class T_DLLCLASS Pipeline : public Object
{
	T_RTTI_CLASS(Pipeline)

public:
	enum BuildReason
	{
		BrNone = 0,					/*!< No reason, buildOutput shouldn't even be called. */
		BrDependencyModified = 1,	/*!< Dependent resource has been modified. */
		BrSourceModified = 2,		/*!< Source asset has been modified. */
		BrForced = 4				/*!< Forced build issued, must build even if output is up-to-date. */
	};

	/*! \brief Create pipeline.
	 *
	 * \param settings Pipeline settings.
	 * \return True if pipeline created successfully.
	 */
	virtual bool create(const Settings* settings) = 0;

	/*! \brief Destroy pipeline. */
	virtual void destroy() = 0;

	/*! \brief Get pipeline version. */
	virtual uint32_t getVersion() const = 0;

	/*! \brief Get pipeline asset types. */
	virtual TypeSet getAssetTypes() const = 0;

	/*! \brief Build dependencies from source asset.
	 *
	 * \param pipelineManager Pipeline manager requesting dependencies.
	 * \param sourceAsset Source asset object.
	 * \param outBuildParams User defined parameter object; passed into buildOutput(...).
	 * \param True if successful.
	 */
	virtual bool buildDependencies(
		PipelineManager* pipelineManager,
		const Serializable* sourceAsset,
		Ref< const Object >& outBuildParams
	) const = 0;

	/*! \brief Build output asset.
	 *
	 * \param pipelineManager Pipeline manager issuing build.
	 * \param sourceAsset Source asset object.
	 * \param buildParams User defined parameter object.
	 * \param outputPath Output path in database.
	 * \param outputGuid Output instance guid.
	 * \param reason Reason as why this asset needs to be rebuilt @sa BuildReason
	 * \return True if successful.
	 */
	virtual bool buildOutput(
		PipelineManager* pipelineManager,
		const Serializable* sourceAsset,
		const Object* buildParams,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		uint32_t reason
	) const = 0;
};

	}
}

#endif	// traktor_editor_Pipeline_H
