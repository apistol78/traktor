#ifndef traktor_render_ShaderPipeline_H
#define traktor_render_ShaderPipeline_H

#include "Core/Heap/Ref.h"
#include "Editor/Pipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class RenderSystem;

class T_DLLCLASS ShaderPipeline : public editor::Pipeline
{
	T_RTTI_CLASS(ShaderPipeline)

public:
	ShaderPipeline();

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
		Object* sourceAsset,
		const Object* buildParams,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		uint32_t reason
	) const;

private:
	Ref< RenderSystem > m_renderSystem;
	int32_t m_optimize;
	bool m_validate;
	bool m_debugCompleteGraphs;
	std::wstring m_debugPath;
};

	}
}

#endif	// traktor_render_ShaderPipeline_H
