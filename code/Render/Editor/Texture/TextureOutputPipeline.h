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
		const editor::IPipelineDependencySet* dependencySet,
		const editor::PipelineDependency* dependency,
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
	enum CompressionMethod
	{
		CmNone,
		CmDXTn,
		CmPVRTC
	};

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
