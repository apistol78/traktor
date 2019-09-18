#include "Core/Math/Winding3.h"
#include "Model/Model.h"
#include "Render/PrimitiveRenderer.h"
#include "Shape/Editor/Solid/PrimitiveEntity.h"
#include "Shape/Editor/Solid/PrimitiveEntityData.h"
#include "Shape/Editor/Solid/SolidEntity.h"
#include "Shape/Editor/Solid/SolidEntityData.h"
#include "Shape/Editor/Solid/SolidEntityEditor.h"
#include "Scene/Editor/EntityAdapter.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SolidEntityEditor", SolidEntityEditor, scene::DefaultEntityEditor)

SolidEntityEditor::SolidEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter)
:	scene::DefaultEntityEditor(context, entityAdapter)
{
}

bool SolidEntityEditor::isPickable() const
{
	return false;
}

bool SolidEntityEditor::isGroup() const
{
	return true;
}

bool SolidEntityEditor::addChildEntity(traktor::scene::EntityAdapter* childEntityAdapter) const
{
	SolidEntityData* solidEntityData = mandatory_non_null_type_cast< SolidEntityData* >(getEntityAdapter()->getEntityData());

	PrimitiveEntityData* childEntityData = dynamic_type_cast< PrimitiveEntityData* >(childEntityAdapter->getEntityData());
    if (!childEntityData)
        return false;

	solidEntityData->addEntityData(childEntityData);
	return true;
}

bool SolidEntityEditor::removeChildEntity(traktor::scene::EntityAdapter* childEntityAdapter) const
{
	SolidEntityData* solidEntityData = mandatory_non_null_type_cast< SolidEntityData* >(getEntityAdapter()->getEntityData());

	PrimitiveEntityData* childEntityData = dynamic_type_cast< PrimitiveEntityData* >(childEntityAdapter->getEntityData());
    if (!childEntityData)
        return false;

	solidEntityData->removeEntityData(childEntityData);
	return true;
}

void SolidEntityEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	SolidEntity* solidEntity = dynamic_type_cast< SolidEntity* >(getEntityAdapter()->getEntity());
	if (!solidEntity)
		return;

	Winding3 winding;

	RefArray< PrimitiveEntity > primitiveEntities;
	solidEntity->getEntitiesOf< PrimitiveEntity >(primitiveEntities);
	for (auto primitiveEntity : primitiveEntities)
	{
		const model::Model* model = primitiveEntity->getModel();
		if (!model)
			continue;

		const auto& vertices = model->getVertices();
		const auto& positions = model->getPositions();

		for (const auto& polygon : model->getPolygons())
		{
			winding.clear();
			for (uint32_t i = 0; i < polygon.getVertexCount(); ++i)
			{
				const auto& vertex = vertices[polygon.getVertex(i)];
				const auto& position = positions[vertex.getPosition()];
				winding.push(position);
			}
			for (uint32_t i = 0; i < winding.size(); ++i)
			{
				uint32_t j = (i + 1) % winding.size();
				primitiveRenderer->drawLine(
					primitiveEntity->getTransform() * winding[i],
					primitiveEntity->getTransform() * winding[j],
					Color4ub(180, 180, 255, 100)
				);
			}			
		}
	}
}

	}
}
