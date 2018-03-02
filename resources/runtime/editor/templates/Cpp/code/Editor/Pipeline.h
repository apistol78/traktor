#ifndef Pipeline_H
#define Pipeline_H

#include <Editor/IPipeline.h>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MYCPPTEST_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

class T_DLLCLASS Pipeline : public traktor::editor::IPipeline
{
	T_RTTI_CLASS;

public:
	virtual bool create(const traktor::editor::IPipelineSettings* settings) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual traktor::TypeInfoSet getAssetTypes() const T_OVERRIDE T_FINAL;

	virtual bool buildDependencies(
		traktor::editor::IPipelineDepends* pipelineDepends,
		const traktor::db::Instance* sourceInstance,
		const traktor::ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const traktor::Guid& outputGuid
	) const T_OVERRIDE T_FINAL;

	virtual bool buildOutput(
		traktor::editor::IPipelineBuilder* pipelineBuilder,
		const traktor::editor::IPipelineDependencySet* dependencySet,
		const traktor::editor::PipelineDependency* dependency,
		const traktor::db::Instance* sourceInstance,
		const traktor::ISerializable* sourceAsset,
		uint32_t sourceAssetHash,
		const std::wstring& outputPath,
		const traktor::Guid& outputGuid,
		const traktor::Object* buildParams,
		uint32_t reason
	) const T_OVERRIDE T_FINAL;

	virtual traktor::Ref< traktor::ISerializable > buildOutput(
		traktor::editor::IPipelineBuilder* pipelineBuilder,
		const traktor::ISerializable* sourceAsset
	) const T_OVERRIDE T_FINAL;
};

#endif	// Pipeline_H
