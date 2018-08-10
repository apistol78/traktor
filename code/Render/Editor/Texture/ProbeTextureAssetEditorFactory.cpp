/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Editor/Texture/ProbeTextureAsset.h"
#include "Render/Editor/Texture/ProbeTextureAssetEditor.h"
#include "Render/Editor/Texture/ProbeTextureAssetEditorFactory.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProbeTextureAssetEditorFactory", 0, ProbeTextureAssetEditorFactory, editor::IObjectEditorFactory)

const TypeInfoSet ProbeTextureAssetEditorFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ProbeTextureAsset >());
	return typeSet;
}

bool ProbeTextureAssetEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	outDependencies.insert(Guid(L"{2F69EAE9-FA20-3244-9B8C-C803E538C19F}"));
	return true;
}

Ref< editor::IObjectEditor > ProbeTextureAssetEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return new ProbeTextureAssetEditor(editor);
}

void ProbeTextureAssetEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

	}
}
