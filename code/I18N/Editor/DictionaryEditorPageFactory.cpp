#include "I18N/Editor/DictionaryEditorPageFactory.h"
#include "I18N/Editor/DictionaryEditorPage.h"
#include "I18N/Dictionary.h"

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

Ref< editor::IEditorPage > DictionaryEditorPageFactory::createEditorPage(editor::IEditor* editor) const
{
	return new DictionaryEditorPage(editor);
}

void DictionaryEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

	}
}
