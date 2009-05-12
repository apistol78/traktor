#include "Editor/App/DefaultObjectEditorFactory.h"
#include "Editor/App/DefaultObjectEditor.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.editor.DefaultObjectEditorFactory", DefaultObjectEditorFactory, ObjectEditorFactory)

const TypeSet DefaultObjectEditorFactory::getEditableTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< Serializable >());
	return typeSet;
}

ObjectEditor* DefaultObjectEditorFactory::createObjectEditor(Editor* editor) const
{
	return gc_new< DefaultObjectEditor >(editor);
}

	}
}
