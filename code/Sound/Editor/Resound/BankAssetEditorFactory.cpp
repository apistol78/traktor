/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Sound/Editor/Resound/BankAsset.h"
#include "Sound/Editor/Resound/BankAssetEditor.h"
#include "Sound/Editor/Resound/BankAssetEditorFactory.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.BankAssetEditorFactory", 0, BankAssetEditorFactory, editor::IObjectEditorFactory)

const TypeInfoSet BankAssetEditorFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< BankAsset >());
	return typeSet;
}

bool BankAssetEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return true;
}

Ref< editor::IObjectEditor > BankAssetEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return new BankAssetEditor(editor);
}

void BankAssetEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

	}
}
