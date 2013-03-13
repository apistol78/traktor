#ifndef traktor_animation_PoseControllerPipeline_H
#define traktor_animation_PoseControllerPipeline_H

#include "Editor/DefaultPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

class T_DLLCLASS PoseControllerPipeline : public editor::DefaultPipeline
{
	T_RTTI_CLASS;

public:
	virtual TypeInfoSet getAssetTypes() const;

	virtual bool buildDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid
	) const;
};

	}
}

#endif	// traktor_animation_PoseControllerPipeline_H
