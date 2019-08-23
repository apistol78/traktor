#include "Scene/Editor/SceneEditorContext.h"
#include "Shape/Editor/Solid/PrimitiveEditModifier.h"
#include "Shape/Editor/Solid/SolidEditorPlugin.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SolidEditorPlugin", SolidEditorPlugin, scene::ISceneEditorPlugin)

SolidEditorPlugin::SolidEditorPlugin(scene::SceneEditorContext* context)
:   m_context(context)
{
}

bool SolidEditorPlugin::create(ui::Widget* parent, ui::ToolBar* toolBar)
{
    return true;
}

bool SolidEditorPlugin::handleCommand(const ui::Command& command)
{
    if (command == L"Shape.Editor.EditPrimitive")
    {
        m_context->setPlaying(false);
        m_context->setModifier(new PrimitiveEditModifier(m_context));
        return true;
    }
    else
        return false;
}

    }
}