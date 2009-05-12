#ifndef traktor_scene_SceneEditorPageFactory_H
#define traktor_scene_SceneEditorPageFactory_H

#include "Editor/EditorPageFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

class T_DLLCLASS SceneEditorPageFactory : public editor::EditorPageFactory
{
	T_RTTI_CLASS(SceneEditorPageFactory)

public:
	virtual const TypeSet getEditableTypes() const;

	virtual editor::EditorPage* createEditorPage(editor::Editor* editor) const;

	virtual void getCommands(std::list< ui::Command >& outCommands) const;
};

	}
}

#endif	// traktor_scene_SceneEditorPageFactory_H
