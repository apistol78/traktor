#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Shape/Editor/MeshModelGenerator.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.MeshModelGenerator", 0, MeshModelGenerator, IModelGenerator)

TypeInfoSet MeshModelGenerator::getSupportedTypes() const
{
    return makeTypeInfoSet< mesh::MeshComponentData >();
}

Ref< model::Model > MeshModelGenerator::createModel(
    editor::IPipelineBuilder* pipelineBuilder,
    const Object* source
) const
{
	const mesh::MeshComponentData* meshComponentData = mandatory_non_null_type_cast< const mesh::MeshComponentData* >(source);
    return nullptr;
}

Ref< Object > MeshModelGenerator::modifyOutput(
    editor::IPipelineBuilder* pipelineBuilder,
    const Object* source,
    const Guid& lightmapId,
    const model::Model* model
) const
{
	const mesh::MeshComponentData* meshComponentData = mandatory_non_null_type_cast< const mesh::MeshComponentData* >(source);
    return nullptr;
}

    }
}