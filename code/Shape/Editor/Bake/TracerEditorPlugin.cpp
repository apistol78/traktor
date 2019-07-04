#include "Shape/Editor/Bake/BakePipelineOperator.h"
#include "Shape/Editor/Bake/TracerEditorPlugin.h"
#include "Shape/Editor/Bake/TracerProcessor.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.TracerEditorPlugin", TracerEditorPlugin, editor::IEditorPlugin)

TracerEditorPlugin::TracerEditorPlugin(editor::IEditor* editor)
:   m_editor(editor)
{
}

bool TracerEditorPlugin::create(ui::Widget* parent, editor::IEditorPageSite* site)
{
    BakePipelineOperator::setTracerProcessor(new TracerProcessor(m_editor));
    return true;
}

void TracerEditorPlugin::destroy()
{
    BakePipelineOperator::setTracerProcessor(nullptr);
}

bool TracerEditorPlugin::handleCommand(const ui::Command& command, bool result)
{
    return false;
}

void TracerEditorPlugin::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void TracerEditorPlugin::handleWorkspaceOpened()
{
}

void TracerEditorPlugin::handleWorkspaceClosed()
{
}

    }
}