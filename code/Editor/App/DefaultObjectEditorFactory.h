#ifndef traktor_editor_DefaultObjectEditorFactory_H
#define traktor_editor_DefaultObjectEditorFactory_H

#include "Editor/IObjectEditorFactory.h"

namespace traktor
{
	namespace editor
	{

class DefaultObjectEditorFactory : public IObjectEditorFactory
{
	T_RTTI_CLASS(DefaultObjectEditorFactory)

public:
	virtual const TypeSet getEditableTypes() const;

	virtual IObjectEditor* createObjectEditor(IEditor* editor) const;
};

	}
}

#endif	// traktor_editor_DefaultObjectEditorFactory_H
