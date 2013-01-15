#include "Terrain/Editor/OceanEntityEditor.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.OceanEntityEditor", OceanEntityEditor, scene::DefaultEntityEditor)

OceanEntityEditor::OceanEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter)
:	scene::DefaultEntityEditor(context, entityAdapter)
{
}


bool OceanEntityEditor::handleCommand(const ui::Command& command)
{
	return false;
}

void OceanEntityEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
}

	}
}
