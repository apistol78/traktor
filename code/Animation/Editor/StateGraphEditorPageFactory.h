#ifndef traktor_animation_StateGraphEditorPageFactory_H
#define traktor_animation_StateGraphEditorPageFactory_H

#include "Editor/EditorPageFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

class T_DLLCLASS StateGraphEditorPageFactory : public editor::EditorPageFactory
{
	T_RTTI_CLASS(StateGraphEditorPageFactory)

public:
	virtual const TypeSet getEditableTypes() const;

	virtual editor::EditorPage* createEditorPage(editor::Editor* editor) const;

	virtual void getCommands(std::list< ui::Command >& outCommands) const;
};

	}
}

#endif	// traktor_animation_StateGraphEditorPageFactory_H
