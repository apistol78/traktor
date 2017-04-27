/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Editor/RenderEditorPlugin.h"
#include "Render/Editor/RenderEditorPluginFactory.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderEditorPluginFactory", 0, RenderEditorPluginFactory, editor::IEditorPluginFactory)

void RenderEditorPluginFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< editor::IEditorPlugin > RenderEditorPluginFactory::createEditorPlugin(editor::IEditor* editor) const
{
	return new RenderEditorPlugin(editor);
}

	}
}
