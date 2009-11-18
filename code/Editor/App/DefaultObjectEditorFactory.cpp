#include "Editor/App/DefaultObjectEditorFactory.h"
#include "Editor/App/DefaultObjectEditor.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.DefaultObjectEditorFactory", 0, DefaultObjectEditorFactory, IObjectEditorFactory)

const TypeInfoSet DefaultObjectEditorFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ISerializable >());
	return typeSet;
}

Ref< IObjectEditor > DefaultObjectEditorFactory::createObjectEditor(IEditor* editor) const
{
	return new DefaultObjectEditor(editor);
}

	}
}
