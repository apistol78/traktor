#ifndef traktor_scene_ScenePipeline_H
#define traktor_scene_ScenePipeline_H

#include "Editor/IPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

class T_DLLCLASS ScenePipeline : public editor::IPipeline
{
	T_RTTI_CLASS;

public:
	ScenePipeline();

	virtual bool create(const editor::IPipelineSettings* settings);

	virtual void destroy();

	virtual TypeInfoSet getAssetTypes() const;

	virtual bool buildDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid
	) const;

	virtual bool buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		uint32_t sourceAssetHash,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		const Object* buildParams,
		uint32_t reason
	) const;

	virtual Ref< ISerializable > buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const ISerializable* sourceAsset
	) const;

private:
	bool m_targetEditor;
	bool m_suppressLinearLighting;
	bool m_suppressDepthPass;
	bool m_suppressPostProcess;
	int32_t m_shadowMapSizeDenom;
	int32_t m_shadowMapMaxSlices;
};

	}
}

#endif	// traktor_scene_ScenePipeline_H
