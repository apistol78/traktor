#ifndef traktor_spray_EffectPipeline_H
#define traktor_spray_EffectPipeline_H

#include "Editor/IPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

class T_DLLCLASS EffectPipeline : public editor::IPipeline
{
	T_RTTI_CLASS(EffectPipeline)

public:
	virtual bool create(const editor::Settings* settings);

	virtual void destroy();

	virtual uint32_t getVersion() const;

	virtual TypeSet getAssetTypes() const;

	virtual bool buildDependencies(
		editor::PipelineManager* pipelineManager,
		const db::Instance* sourceInstance,
		const Serializable* sourceAsset,
		Ref< const Object >& outBuildParams
	) const;

	virtual bool buildOutput(
		editor::PipelineManager* pipelineManager,
		const Serializable* sourceAsset,
		const Object* buildParams,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		uint32_t reason
	) const;
};

	}
}

#endif	// traktor_spray_EffectPipeline_H
