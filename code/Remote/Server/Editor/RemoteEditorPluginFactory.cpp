#include "Remote/Server/Editor/RemoteEditorPlugin.h"
#include "Remote/Server/Editor/RemoteEditorPluginFactory.h"

namespace traktor
{
	namespace remote
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.remote.RemoteEditorPluginFactory", 0, RemoteEditorPluginFactory, editor::IEditorPluginFactory)

void RemoteEditorPluginFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< editor::IEditorPlugin > RemoteEditorPluginFactory::createEditorPlugin(editor::IEditor* editor) const
{
	return new RemoteEditorPlugin(editor);
}

	}
}
