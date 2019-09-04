#pragma once

#include "Shape/Editor/IModelGenerator.h"

namespace traktor
{
    namespace shape
    {

class SolidModelGenerator : public IModelGenerator
{
    T_RTTI_CLASS;

public:
    virtual TypeInfoSet getSupportedTypes() const override final;

    virtual Ref< model::Model > createModel(
        editor::IPipelineBuilder* pipelineBuilder,
		const std::wstring& assetPath,
        const Object* source
    ) const override final;

    virtual Ref< Object > modifyOutput(
        editor::IPipelineBuilder* pipelineBuilder,
		const std::wstring& assetPath,
        const Object* source,
        const Guid& lightmapId,
        const model::Model* model
    ) const override final;
};

    }
}