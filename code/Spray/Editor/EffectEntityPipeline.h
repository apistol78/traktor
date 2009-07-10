#ifndef traktor_spray_EffectEntityPipeline_H
#define traktor_spray_EffectEntityPipeline_H

#include "World/Editor/EntityPipeline.h"

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

class T_DLLCLASS EffectEntityPipeline : public world::EntityPipeline
{
	T_RTTI_CLASS(EffectEntityPipeline)

public:
	virtual TypeSet getAssetTypes() const;

	virtual bool buildDependencies(
		editor::PipelineManager* pipelineManager,
		const db::Instance* sourceInstance,
		const Serializable* sourceAsset,
		Ref< const Object >& outBuildParams
	) const;
};

	}
}

#endif	// traktor_spray_EffectEntityPipeline_H
