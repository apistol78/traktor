#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Shape/Editor/Spline/ControlPointComponentEditor.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.ControlPointComponentEditor", ControlPointComponentEditor, scene::DefaultComponentEditor)

ControlPointComponentEditor::ControlPointComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData)
:	scene::DefaultComponentEditor(context, entityAdapter, componentData)
{
}

void ControlPointComponentEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	primitiveRenderer->pushDepthState(false, false, false);

	const auto& T = m_entityAdapter->getTransform();
	primitiveRenderer->drawSolidPoint(T.translation(), 4.0f, Color4ub(255, 255, 255, 200));

	primitiveRenderer->popDepthState();
}

	}
}
