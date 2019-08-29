#include "Model/Model.h"
#include "Shape/Editor/Solid/IShape.h"
#include "Shape/Editor/Solid/PrimitiveEntityData.h"
#include "Shape/Editor/Solid/SolidEntityData.h"
#include "Shape/Editor/Solid/SolidModelGenerator.h"
#include "Shape/Editor/Solid/Utilities.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.SolidModelGenerator", 0, SolidModelGenerator, IModelGenerator)

TypeInfoSet SolidModelGenerator::getSupportedTypes() const
{
    return makeTypeInfoSet< SolidEntityData >();
}

Ref< model::Model > SolidModelGenerator::createModel(const Object* source) const
{
	const SolidEntityData* solidEntityData = mandatory_non_null_type_cast< const SolidEntityData* >(source);
    
    // Get all primitive entities with shape.
    RefArray< const PrimitiveEntityData > primitiveEntityDatas;
    for (auto entityData : solidEntityData->getEntityData())
    {
        if (const auto primitiveEntityData = dynamic_type_cast< const PrimitiveEntityData* >(entityData))
        {
            if (primitiveEntityData->getShape() != nullptr)
                primitiveEntityDatas.push_back(primitiveEntityData);
        }
    }

    // Merge all windings into a single group of windings.
    AlignedVector< Winding3 > outputWindings;

    auto it = primitiveEntityDatas.begin();
    if (it != primitiveEntityDatas.end())
    {
        (*it)->getShape()->createWindings(outputWindings);
        outputWindings = transform(outputWindings, (*it)->getTransform());

        for (++it; it != primitiveEntityDatas.end(); ++it)
        {
            AlignedVector< Winding3 > windings;
            (*it)->getShape()->createWindings(windings);
            if (windings.empty())
                continue;

            windings = transform(windings, (*it)->getTransform());

            switch ((*it)->getOperation())
            {
            case BooleanOperation::BoUnion:
                {
                    auto result = unioon(outputWindings, windings);
                    outputWindings.swap(result);
                }
                break;

            case BooleanOperation::BoIntersection:
                {
                    auto result = intersection(outputWindings, windings);
                    outputWindings.swap(result);
                }
                break;

            case BooleanOperation::BoDifference:
                {
                    auto result = difference(outputWindings, windings);
                    outputWindings.swap(result);
                }
                break;
            }
        }
    }

    // Create model from windings.
    Ref< model::Model > outputModel = new model::Model();
    outputModel->addMaterial(model::Material(
        L"Default"
    ));
    for (const auto& winding : outputWindings)
    {
		Plane pl;
		if (!winding.getPlane(pl))
			continue;

		uint32_t normal = outputModel->addUniqueNormal(pl.normal());

        model::Polygon polygon;
        polygon.setMaterial(0);
		polygon.setNormal(normal);
        for (const auto& vx : winding.get())
        {
            model::Vertex vertex;
            vertex.setPosition(outputModel->addUniquePosition(vx));
			vertex.setNormal(normal);
            polygon.addVertex(outputModel->addUniqueVertex(vertex));
        }
        outputModel->addUniquePolygon(polygon);
    }

    return outputModel;
}

    }
}