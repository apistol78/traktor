#include "I18N/Editor/DictionaryEditorPageFactory.h"
#include "I18N/Editor/DictionaryEditorPage.h"
#include "I18N/Dictionary.h"

namespace traktor
{
	namespace i18n
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.i18n.DictionaryEditorPageFactory", DictionaryEditorPageFactory, editor::EditorPageFactory)

const TypeSet DictionaryEditorPageFactory::getEditableTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< Dictionary >());
	return typeSet;
}

editor::EditorPage* DictionaryEditorPageFactory::createEditorPage(editor::Editor* editor) const
{
	return gc_new< DictionaryEditorPage >(editor);
}

void DictionaryEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

	}
}
