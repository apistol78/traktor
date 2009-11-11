#ifndef traktor_editor_IObjectEditorFactory_H
#define traktor_editor_IObjectEditorFactory_H

#include "Core/Object.h"
#include "Core/Heap/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IEditor;
class IObjectEditor;

/*! \brief Interface used by Editor to create object editors.
 * \ingroup Editor
 */
class T_DLLCLASS IObjectEditorFactory : public Object
{
	T_RTTI_CLASS(IObjectEditorFactory)

public:
	virtual const TypeSet getEditableTypes() const = 0;

	virtual Ref< IObjectEditor > createObjectEditor(IEditor* editor) const = 0;
};

	}
}

#endif	// traktor_editor_IObjectEditorFactory_H
