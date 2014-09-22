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

	virtual bool create(const editor::IPipelineSettings* settings);

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

private:
	std::wstring m_assetPath;
	bool m_targetEditor;
};

	}
}

#endif	// traktor_illuminate_IlluminateEntityPipeline_H
