#ifndef traktor_uikit_WidgetPreviewEditorFactory_H
#define traktor_uikit_WidgetPreviewEditorFactory_H

#include "Editor/IObjectEditorFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UIKIT_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace uikit
	{

class T_DLLCLASS WidgetPreviewEditorFactory : public editor::IObjectEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEditableTypes() const;

	virtual bool needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const;

	virtual Ref< editor::IObjectEditor > createObjectEditor(editor::IEditor* editor) const;

	virtual void getCommands(std::list< ui::Command >& outCommands) const;	
};

	}
}

#endif	// traktor_uikit_WidgetPreviewEditorFactory_H
