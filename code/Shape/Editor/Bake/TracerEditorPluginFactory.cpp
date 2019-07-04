#include "Shape/Editor/Bake/TracerEditorPlugin.h"
#include "Shape/Editor/Bake/TracerEditorPluginFactory.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.TracerEditorPluginFactory", 0, TracerEditorPluginFactory, editor::IEditorPluginFactory)

void TracerEditorPluginFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< editor::IEditorPlugin > TracerEditorPluginFactory::createEditorPlugin(editor::IEditor* editor) const
{
    return new TracerEditorPlugin(editor);
}

    }
}