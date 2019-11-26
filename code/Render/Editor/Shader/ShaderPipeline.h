#pragma once

#include "Core/Thread/Semaphore.h"
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

class IProgramCompiler;
class IProgramHints;

class T_DLLCLASS ShaderPipeline : public editor::IPipeline
{
	T_RTTI_CLASS;

public:
	ShaderPipeline();

	virtual bool create(const editor::IPipelineSettings* settings) override final;

	virtual void destroy() override final;

	virtual TypeInfoSet getAssetTypes() const override final;

	virtual bool buildDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid
	) const override final;

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
	) const override final;

	virtual Ref< ISerializable > buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const Object* buildParams
	) const override final;

private:
	std::wstring m_programCompilerTypeName;
	std::wstring m_programCachePath;
	mutable Semaphore m_programCompilerLock;
	mutable Ref< IProgramCompiler > m_programCompiler;
	mutable Ref< ProgramCache > m_programCache;
	Ref< IProgramHints > m_programHints;
	std::set< std::wstring > m_includeOnlyTechniques;
	Ref< const PropertyGroup > m_compilerSettings;
	std::wstring m_platform;
	bool m_frequentUniformsAsLinear;
	int32_t m_optimize;
	bool m_validate;
	bool m_debugCompleteGraphs;
	std::wstring m_debugPath;
	bool m_editor;

	IProgramCompiler* getProgramCompiler() const;

	ProgramCache* getProgramCache() const;
};

	}
}

