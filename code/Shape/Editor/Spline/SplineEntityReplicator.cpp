#include "Model/Model.h"
#include "Shape/Editor/Spline/SplineEntityData.h"
#include "Shape/Editor/Spline/SplineEntityReplicator.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.SplineEntityReplicator", 0, SplineEntityReplicator, scene::IEntityReplicator)

TypeInfoSet SplineEntityReplicator::getSupportedTypes() const
{
    return makeTypeInfoSet< SplineEntityData >();
}

Ref< model::Model > SplineEntityReplicator::createModel(
    editor::IPipelineBuilder* pipelineBuilder,
    const std::wstring& assetPath,
    const Object* source
) const
{
	const SplineEntityData* splineEntityData = mandatory_non_null_type_cast< const SplineEntityData* >(source);

    Ref< model::Model > outputModel = new model::Model();

    return outputModel;
}

Ref< Object > SplineEntityReplicator::modifyOutput(
    editor::IPipelineBuilder* pipelineBuilder,
    const std::wstring& assetPath,
    const Object* source,
    const model::Model* model
) const
{
    return nullptr;
}

    }
}
