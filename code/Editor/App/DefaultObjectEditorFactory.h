#ifndef traktor_editor_DefaultObjectEditorFactory_H
#define traktor_editor_DefaultObjectEditorFactory_H

#include "Editor/IObjectEditorFactory.h"

namespace traktor
{
	namespace editor
	{

class DefaultObjectEditorFactory : public IObjectEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEditableTypes() const;

	virtual Ref< IObjectEditor > createObjectEditor(IEditor* editor) const;
};

	}
}

#endif	// traktor_editor_DefaultObjectEditorFactory_H
