#ifndef traktor_illuminate_IlluminateEntityPipeline_H
#define traktor_illuminate_IlluminateEntityPipeline_H

#include "World/Editor/EntityPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ILLUMINATE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace illuminate
	{

class T_DLLCLASS IlluminateEntityPipeline : public world::EntityPipeline
{
	T_RTTI_CLASS;

public:
	IlluminateEntityPipeline();

	virtual bool create(const editor::IPipelineSettings* settings) T_OVERRIDE T_FINAL;

	virtual TypeInfoSet getAssetTypes() const T_OVERRIDE T_FINAL;

	virtual bool buildDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid
	) const T_OVERRIDE T_FINAL;

	virtual Ref< ISerializable > buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const ISerializable* sourceAsset
	) const T_OVERRIDE T_FINAL;

private:
	std::wstring m_assetPath;
	bool m_build;
};

	}
}

#endif	// traktor_illuminate_IlluminateEntityPipeline_H
