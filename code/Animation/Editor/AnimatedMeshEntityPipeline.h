#ifndef traktor_animation_AnimatedMeshEntityPipeline_H
#define traktor_animation_AnimatedMeshEntityPipeline_H

#include "World/Editor/EntityPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

class T_DLLCLASS AnimatedMeshEntityPipeline : public world::EntityPipeline
{
	T_RTTI_CLASS(AnimatedMeshEntityPipeline)

public:
	virtual TypeSet getAssetTypes() const;

	virtual bool buildDependencies(
		editor::PipelineManager* pipelineManager,
		const Object* sourceAsset,
		Ref< const Object >& outBuildParams
	) const;
};

	}
}

#endif	// traktor_animation_AnimatedMeshEntityPipeline_H
