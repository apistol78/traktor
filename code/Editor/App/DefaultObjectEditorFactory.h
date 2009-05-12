#ifndef traktor_editor_DefaultObjectEditorFactory_H
#define traktor_editor_DefaultObjectEditorFactory_H

#include "Editor/ObjectEditorFactory.h"

namespace traktor
{
	namespace editor
	{

class DefaultObjectEditorFactory : public ObjectEditorFactory
{
	T_RTTI_CLASS(DefaultObjectEditorFactory)

public:
	virtual const TypeSet getEditableTypes() const;

	virtual ObjectEditor* createObjectEditor(Editor* editor) const;
};

	}
}

#endif	// traktor_editor_DefaultObjectEditorFactory_H
