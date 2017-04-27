/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "I18N/Dictionary.h"
#include "I18N/Editor/DictionaryEditorPage.h"
#include "I18N/Editor/DictionaryEditorPageFactory.h"

namespace traktor
{
	namespace i18n
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.i18n.DictionaryEditorPageFactory", 0, DictionaryEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet DictionaryEditorPageFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Dictionary >());
	return typeSet;
}

bool DictionaryEditorPageFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return false;
}

Ref< editor::IEditorPage > DictionaryEditorPageFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new DictionaryEditorPage(editor, site, document);
}

void DictionaryEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

	}
}
