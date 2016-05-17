#ifndef traktor_render_TextureOutputPipeline_H
#define traktor_render_TextureOutputPipeline_H

#include "Editor/IPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class T_DLLCLASS TextureOutputPipeline : public editor::IPipeline
{
	T_RTTI_CLASS;

public:
	TextureOutputPipeline();

	virtual bool create(const editor::IPipelineSettings* settings) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual TypeInfoSet getAssetTypes() const T_OVERRIDE T_FINAL;

	virtual bool buildDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid
	) const T_OVERRIDE T_FINAL;

	virtual bool buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const editor::IPipelineDependencySet* dependencySet,
		const editor::PipelineDependency* dependency,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		uint32_t sourceAssetHash,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		const Object* buildParams,
		uint32_t reason
	) const T_OVERRIDE T_FINAL;

	virtual Ref< ISerializable > buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const ISerializable* sourceAsset
	) const T_OVERRIDE T_FINAL;

private:
	enum CompressionMethod
	{
		CmNone,
		CmDXTn,
		CmPVRTC,
		CmETC1
	};

	bool m_generateMipsThread;
	int32_t m_skipMips;
	int32_t m_clampSize;
	CompressionMethod m_compressionMethod;
	int32_t m_compressionQuality;
	float m_gamma;
	bool m_sRGB;
};

	}
}

#endif	// traktor_render_TextureOutputPipeline_H
