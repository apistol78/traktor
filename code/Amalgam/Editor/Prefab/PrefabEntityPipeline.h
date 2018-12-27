#ifndef traktor_amalgam_PrefabEntityPipeline_H
#define traktor_amalgam_PrefabEntityPipeline_H

#include "World/Editor/EntityPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class T_DLLCLASS PrefabEntityPipeline : public world::EntityPipeline
{
	T_RTTI_CLASS;

public:
	PrefabEntityPipeline();

	virtual bool create(const editor::IPipelineSettings* settings) override final;

	virtual TypeInfoSet getAssetTypes() const override final;

	virtual bool buildDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid
	) const override final;

	virtual Ref< ISerializable > buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const ISerializable* sourceAsset
	) const override final;

private:
	std::wstring m_assetPath;
	bool m_targetEditor;
	mutable std::set< Guid > m_usedGuids;
};

	}
}

#endif	// traktor_amalgam_PrefabEntityPipeline_H
