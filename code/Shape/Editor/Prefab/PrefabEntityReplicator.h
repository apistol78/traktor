#pragma once

#include "Scene/Editor/IEntityReplicator.h"

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

class T_DLLCLASS PrefabEntityReplicator : public scene::IEntityReplicator
{
    T_RTTI_CLASS;

public:
    virtual bool create(const editor::IPipelineSettings* settings) override final;

    virtual TypeInfoSet getSupportedTypes() const override final;

    virtual bool addDependencies(
        editor::IPipelineDepends* pipelineDepends,
        const world::EntityData* entityData,
        const world::IEntityComponentData* componentData
    ) const override final;

    virtual Ref< model::Model > createModel(
        editor::IPipelineBuilder* pipelineBuilder,
        const world::EntityData* entityData,
        const world::IEntityComponentData* componentData
    ) const override final;

    virtual Ref< Object > modifyOutput(
        editor::IPipelineBuilder* pipelineBuilder,
        const world::EntityData* entityData,
        const world::IEntityComponentData* componentData,
        const model::Model* model,
        const Guid& outputGuid
    ) const override final;

private:
    std::wstring m_assetPath;
    std::wstring m_modelCachePath;
};

    }
}