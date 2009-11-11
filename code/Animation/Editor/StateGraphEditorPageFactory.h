#ifndef traktor_animation_StateGraphEditorPageFactory_H
#define traktor_animation_StateGraphEditorPageFactory_H

#include "Editor/IEditorPageFactory.h"

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

class T_DLLCLASS StateGraphEditorPageFactory : public editor::IEditorPageFactory
{
	T_RTTI_CLASS(StateGraphEditorPageFactory)

public:
	virtual const TypeSet getEditableTypes() const;

	virtual Ref< editor::IEditorPage > createEditorPage(editor::IEditor* editor) const;

	virtual void getCommands(std::list< ui::Command >& outCommands) const;
};

	}
}

#endif	// traktor_animation_StateGraphEditorPageFactory_H
