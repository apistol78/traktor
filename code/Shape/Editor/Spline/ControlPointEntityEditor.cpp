#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Shape/Spline/ControlPointEntityData.h"
#include "Shape/Editor/Spline/ControlPointEntityEditor.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.ControlPointEntityEditor", ControlPointEntityEditor, scene::DefaultEntityEditor)

ControlPointEntityEditor::ControlPointEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter)
:	scene::DefaultEntityEditor(context, entityAdapter)
{
}

void ControlPointEntityEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	const ControlPointEntityData* controlEntityData = checked_type_cast< const ControlPointEntityData* >(getEntityAdapter()->getEntityData());

	primitiveRenderer->pushDepthState(false, false, false);

	const auto& T = controlEntityData->getTransform();
	primitiveRenderer->drawSolidPoint(T.translation(), 4.0f, Color4ub(255, 255, 255, 200));

	primitiveRenderer->popDepthState();
}

	}
}
