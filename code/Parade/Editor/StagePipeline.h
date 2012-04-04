#ifndef traktor_parade_StagePipeline_H
#define traktor_parade_StagePipeline_H

#include "Editor/DefaultPipeline.h"

namespace traktor
{
	namespace parade
	{

class T_DLLEXPORT StagePipeline : public editor::DefaultPipeline
{
	T_RTTI_CLASS;

public:
	virtual TypeInfoSet getAssetTypes() const;

	virtual bool buildDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		Ref< const Object >& outBuildParams
	) const;
};

	}
}

#endif	// traktor_parade_StagePipeline_H
