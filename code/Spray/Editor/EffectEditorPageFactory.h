#ifndef traktor_spray_EffectEditorPageFactory_H
#define traktor_spray_EffectEditorPageFactory_H

#include "Editor/EditorPageFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

class T_DLLCLASS EffectEditorPageFactory : public editor::EditorPageFactory
{
	T_RTTI_CLASS(EffectEditorPageFactory)

public:
	virtual const TypeSet getEditableTypes() const;

	virtual editor::EditorPage* createEditorPage(editor::Editor* editor) const;

	virtual void getCommands(std::list< ui::Command >& outCommands) const;
};

	}
}

#endif	// traktor_spray_EffectEditorPageFactory_H
