#ifndef traktor_i18n_DictionaryEditorPageFactory_H
#define traktor_i18n_DictionaryEditorPageFactory_H

#include "Editor/IEditorPageFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_I18N_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace i18n
	{

class T_DLLCLASS DictionaryEditorPageFactory : public editor::IEditorPageFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEditableTypes() const;

	virtual bool needOutputResources(const TypeInfo& typeInfo) const;

	virtual Ref< editor::IEditorPage > createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const;

	virtual void getCommands(std::list< ui::Command >& outCommands) const;
};

	}
}

#endif	// traktor_i18n_DictionaryEditorPageFactory_H
