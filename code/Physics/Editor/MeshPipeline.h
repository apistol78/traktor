#ifndef traktor_physics_MeshPipeline_H
#define traktor_physics_MeshPipeline_H

#include "Editor/IPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class T_DLLCLASS MeshPipeline : public editor::IPipeline
{
	T_RTTI_CLASS(MeshPipeline)

public:
	virtual bool create(const editor::Settings* settings);

	virtual void destroy();

	virtual uint32_t getVersion() const;

	virtual TypeSet getAssetTypes() const;

	virtual bool buildDependencies(
		editor::PipelineManager* pipelineManager,
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

#endif	// traktor_physics_MeshPipeline_H
