#include "Sound/Editor/Resound/BankAsset.h"
#include "Sound/Editor/Resound/BankAssetEditor.h"
#include "Sound/Editor/Resound/BankAssetEditorFactory.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.BankAssetEditorFactory", 0, BankAssetEditorFactory, editor::IObjectEditorFactory)

const TypeInfoSet BankAssetEditorFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< BankAsset >());
	return typeSet;
}

Ref< editor::IObjectEditor > BankAssetEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return new BankAssetEditor(editor);
}

	}
}
