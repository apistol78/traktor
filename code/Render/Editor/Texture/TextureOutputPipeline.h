#pragma once

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

	virtual bool create(const editor::IPipelineSettings* settings) override final;

	virtual void destroy() override final;

	virtual TypeInfoSet getAssetTypes() const override final;

	virtual bool shouldCache() const override final;

	virtual uint32_t hashAsset(const ISerializable* sourceAsset) const override final;
	
	virtual bool buildDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid
	) const override final;

	virtual bool buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const editor::PipelineDependencySet* dependencySet,
		const editor::PipelineDependency* dependency,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		const Object* buildParams,
		uint32_t reason
	) const override final;

	virtual Ref< ISerializable > buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const Object* buildParams
	) const override final;

private:
	enum CompressionMethod
	{
		CmNone,
		CmDXTn,
		CmPVRTC,
		CmETC1,
		CmASTC
	};

	bool m_generateMipsThread;
	int32_t m_skipMips;
	int32_t m_clampSize;
	CompressionMethod m_compressionMethod;
	int32_t m_compressionQuality;
	float m_gamma;
	bool m_sRGB;
	bool m_compressedData;
};

	}
}

