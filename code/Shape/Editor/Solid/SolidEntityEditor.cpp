#include "Shape/Editor/Solid/PrimitiveEntityData.h"
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
    if (childEntityData)
        return false;

	solidEntityData->addEntityData(childEntityData);
	return true;
}

bool SolidEntityEditor::removeChildEntity(traktor::scene::EntityAdapter* childEntityAdapter) const
{
	SolidEntityData* solidEntityData = mandatory_non_null_type_cast< SolidEntityData* >(getEntityAdapter()->getEntityData());

	PrimitiveEntityData* childEntityData = dynamic_type_cast< PrimitiveEntityData* >(childEntityAdapter->getEntityData());
    if (childEntityData)
        return false;

	solidEntityData->removeEntityData(childEntityData);
	return true;
}

	}
}
