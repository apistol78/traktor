#ifndef traktor_editor_ObjectEditorFactory_H
#define traktor_editor_ObjectEditorFactory_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"

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

class Editor;
class ObjectEditor;

class T_DLLCLASS ObjectEditorFactory : public Object
{
	T_RTTI_CLASS(ObjectEditorFactory)

public:
	virtual const TypeSet getEditableTypes() const = 0;

	virtual ObjectEditor* createObjectEditor(Editor* editor) const = 0;
};

	}
}

#endif	// traktor_editor_ObjectEditorFactory_H
