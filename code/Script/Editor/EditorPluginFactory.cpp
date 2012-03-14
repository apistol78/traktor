#include "Core/Serialization/ISerializable.h"
#include "Script/Editor/EditorPlugin.h"
#include "Script/Editor/EditorPluginFactory.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.EditorPluginFactory", 0, EditorPluginFactory, editor::IEditorPluginFactory)

void EditorPluginFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< editor::IEditorPlugin > EditorPluginFactory::createEditorPlugin(editor::IEditor* editor) const
{
	return new EditorPlugin(editor);
}

	}
}
