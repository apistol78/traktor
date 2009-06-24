#ifndef traktor_scene_SceneEditorPageFactory_H
#define traktor_scene_SceneEditorPageFactory_H

#include "Editor/IEditorPageFactory.h"

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

class T_DLLCLASS SceneEditorPageFactory : public editor::IEditorPageFactory
{
	T_RTTI_CLASS(SceneEditorPageFactory)

public:
	virtual const TypeSet getEditableTypes() const;

	virtual editor::IEditorPage* createEditorPage(editor::IEditor* editor) const;

	virtual void getCommands(std::list< ui::Command >& outCommands) const;
};

	}
}

#endif	// traktor_scene_SceneEditorPageFactory_H
