#ifndef traktor_amalgam_TargetPipeline_H
#define traktor_amalgam_TargetPipeline_H

#include "Editor/IPipeline.h"

namespace traktor
{
	namespace amalgam
	{

class TargetPipeline : public editor::IPipeline
{
	T_RTTI_CLASS;

public:
	virtual bool create(const editor::IPipelineSettings* settings);

	virtual void destroy();

	virtual TypeInfoSet getAssetTypes() const;

	virtual bool buildDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		Ref< const Object >& outBuildParams
	) const;

	virtual bool buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const ISerializable* sourceAsset,
		uint32_t sourceAssetHash,
		const Object* buildParams,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		uint32_t reason
	) const;
};

	}
}

#endif	// traktor_amalgam_TargetPipeline_H
