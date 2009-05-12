#ifndef traktor_animation_SkeletonEditorPageFactory_H
#define traktor_animation_SkeletonEditorPageFactory_H

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

class T_DLLCLASS SkeletonEditorPageFactory : public editor::EditorPageFactory
{
	T_RTTI_CLASS(SkeletonEditorPageFactory)

public:
	virtual const TypeSet getEditableTypes() const;

	virtual editor::EditorPage* createEditorPage(editor::Editor* editor) const;

	virtual void getCommands(std::list< ui::Command >& outCommands) const;
};

	}
}

#endif	// traktor_animation_SkeletonEditorPageFactory_H
