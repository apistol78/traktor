/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/HeightfieldAssetEditor.h"
#include "Heightfield/Editor/HeightfieldAssetEditorFactory.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.HeightfieldAssetEditorFactory", 0, HeightfieldAssetEditorFactory, editor::IObjectEditorFactory)

const TypeInfoSet HeightfieldAssetEditorFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< HeightfieldAsset >());
	return typeSet;
}

bool HeightfieldAssetEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return false;
}

Ref< editor::IObjectEditor > HeightfieldAssetEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return new HeightfieldAssetEditor(editor);
}

void HeightfieldAssetEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

	}
}
