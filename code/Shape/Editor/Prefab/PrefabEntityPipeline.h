#pragma once

#include "World/Editor/EntityPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace shape
	{

class T_DLLCLASS PrefabEntityPipeline : public world::EntityPipeline
{
	T_RTTI_CLASS;

public:
	PrefabEntityPipeline();

	virtual bool create(const editor::IPipelineSettings* settings) override final;

	virtual TypeInfoSet getAssetTypes() const override final;

	virtual Ref< ISerializable > buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const Object* buildParams
	) const override final;

private:
	std::wstring m_assetPath;
	float m_visualMeshSnap;
	float m_collisionMeshSnap;
	bool m_mergeCoplanar;
};

	}
}

