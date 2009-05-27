#ifndef traktor_world_EntityPipeline_H
#define traktor_world_EntityPipeline_H

#include "Editor/Pipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class T_DLLCLASS EntityPipeline : public editor::Pipeline
{
	T_RTTI_CLASS(EntityPipeline)

public:
	virtual bool create(const editor::Settings* settings);

	virtual void destroy();

	virtual uint32_t getVersion() const;

	virtual TypeSet getAssetTypes() const;

	virtual bool buildDependencies(
		editor::PipelineManager* pipelineManager,
		const Object* sourceAsset,
		Ref< const Object >& outBuildParams
	) const;

	virtual bool buildOutput(
		editor::PipelineManager* pipelineManager,
		const Object* sourceAsset,
		const Object* buildParams,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		uint32_t reason
	) const;
};

	}
}

#endif	// traktor_world_EntityPipeline_H
