#ifndef traktor_editor_EditorPageFactory_H
#define traktor_editor_EditorPageFactory_H

#include <list>
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
	namespace ui
	{

class Command;

	}

	namespace editor
	{

class Editor;
class EditorPage;

class T_DLLCLASS EditorPageFactory : public Object
{
	T_RTTI_CLASS(EditorPageFactory)

public:
	virtual const TypeSet getEditableTypes() const = 0;

	virtual EditorPage* createEditorPage(Editor* editor) const = 0;

	virtual void getCommands(std::list< ui::Command >& outCommands) const = 0;
};

	}
}

#endif	// traktor_editor_EditorPageFactory_H
