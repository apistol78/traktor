#ifndef traktor_i18n_DictionaryEditorPageFactory_H
#define traktor_i18n_DictionaryEditorPageFactory_H

#include "Editor/EditorPageFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_I18N_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace i18n
	{

class T_DLLCLASS DictionaryEditorPageFactory : public editor::EditorPageFactory
{
	T_RTTI_CLASS(DictionaryEditorPageFactory)

public:
	virtual const TypeSet getEditableTypes() const;

	virtual editor::EditorPage* createEditorPage(editor::Editor* editor) const;

	virtual void getCommands(std::list< ui::Command >& outCommands) const;
};

	}
}

#endif	// traktor_i18n_DictionaryEditorPageFactory_H
