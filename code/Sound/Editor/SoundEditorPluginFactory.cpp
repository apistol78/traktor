/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Sound/Editor/SoundEditorPlugin.h"
#include "Sound/Editor/SoundEditorPluginFactory.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundEditorPluginFactory", 0, SoundEditorPluginFactory, editor::IEditorPluginFactory)

void SoundEditorPluginFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< editor::IEditorPlugin > SoundEditorPluginFactory::createEditorPlugin(editor::IEditor* editor) const
{
	return new SoundEditorPlugin(editor);
}

	}
}
