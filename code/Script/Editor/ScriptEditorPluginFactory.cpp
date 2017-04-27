/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Script/Editor/ScriptEditorPlugin.h"
#include "Script/Editor/ScriptEditorPluginFactory.h"

namespace traktor
{
	namespace script
	{
	
T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptEditorPluginFactory", 0, ScriptEditorPluginFactory, editor::IEditorPluginFactory)	

void ScriptEditorPluginFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< editor::IEditorPlugin > ScriptEditorPluginFactory::createEditorPlugin(editor::IEditor* editor) const
{
	return new ScriptEditorPlugin(editor);
}

	}
}
