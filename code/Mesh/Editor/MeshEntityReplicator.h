#pragma once

#include "Scene/Editor/IEntityReplicator.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class T_DLLCLASS MeshEntityReplicator : public scene::IEntityReplicator
{
	T_RTTI_CLASS;

public:
	virtual bool create(const editor::IPipelineSettings* settings) override final;

	virtual TypeInfoSet getSupportedTypes() const override final;

	virtual Ref< model::Model > createVisualModel(
		editor::IPipelineBuilder* pipelineBuilder,
		const world::EntityData* entityData,
		const world::IEntityComponentData* componentData
	) const override final;

	virtual Ref< model::Model > createCollisionModel(
		editor::IPipelineBuilder* pipelineBuilder,
		const world::EntityData* entityData,
		const world::IEntityComponentData* componentData
	) const override final;

private:
	std::wstring m_assetPath;
	std::wstring m_modelCachePath;
};

	}
}