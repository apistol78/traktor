#ifndef traktor_spray_EffectEntityPipeline_H
#define traktor_spray_EffectEntityPipeline_H

#include "World/Editor/EntityPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

class T_DLLCLASS EffectEntityPipeline : public world::EntityPipeline
{
	T_RTTI_CLASS;

public:
	virtual TypeInfoSet getAssetTypes() const;

	virtual bool buildDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid
	) const;

	virtual Ref< ISerializable > buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const ISerializable* sourceAsset
	) const;
};

	}
}

#endif	// traktor_spray_EffectEntityPipeline_H
