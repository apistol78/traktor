#include "Runtime/Editor/EditorPluginFactory.h"
#include "Runtime/Editor/EditorPlugin.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.EditorPluginFactory", 0, EditorPluginFactory, editor::IEditorPluginFactory)

void EditorPluginFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< editor::IEditorPlugin > EditorPluginFactory::createEditorPlugin(editor::IEditor* editor) const
{
	return new EditorPlugin(editor);
}

	}
}
