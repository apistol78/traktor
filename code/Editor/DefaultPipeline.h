#ifndef traktor_editor_DefaultPipeline_H
#define traktor_editor_DefaultPipeline_H

#include "Editor/IPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

/*! \brief Default pipeline.
 * \ingroup Editor
 *
 * This pipeline is used for every source instance
 * which hasn't a dedicated pipeline.
 *
 * It copies the source instance to the
 * output database without any modification.
 */
class T_DLLCLASS DefaultPipeline : public IPipeline
{
	T_RTTI_CLASS(DefaultPipeline)

public:
	virtual bool create(const Settings* settings);

	virtual void destroy();

	virtual uint32_t getVersion() const;

	virtual TypeSet getAssetTypes() const;

	virtual bool buildDependencies(
		PipelineManager* pipelineManager,
		const Serializable* sourceAsset,
		Ref< const Object >& outBuildParams
	) const;

	virtual bool buildOutput(
		PipelineManager* pipelineManager,
		const Serializable* sourceAsset,
		const Object* buildParams,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		uint32_t reason
	) const;
};

	}
}

#endif	// traktor_editor_DefaultPipeline_H
