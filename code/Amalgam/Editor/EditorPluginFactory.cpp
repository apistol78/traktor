/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/Editor/EditorPluginFactory.h"
#include "Amalgam/Editor/EditorPlugin.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.EditorPluginFactory", 0, EditorPluginFactory, editor::IEditorPluginFactory)

void EditorPluginFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< editor::IEditorPlugin > EditorPluginFactory::createEditorPlugin(editor::IEditor* editor) const
{
	return new EditorPlugin(editor);
}

	}
}
