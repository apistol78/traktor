/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_script_ScriptEditorPageFactory_H
#define traktor_script_ScriptEditorPageFactory_H

#include "Editor/IEditorPageFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class T_DLLCLASS ScriptEditorPageFactory : public editor::IEditorPageFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEditableTypes() const T_OVERRIDE T_FINAL;

	virtual bool needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const T_OVERRIDE T_FINAL;

	virtual Ref< editor::IEditorPage > createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const T_OVERRIDE T_FINAL;

	virtual void getCommands(std::list< ui::Command >& outCommands) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_script_ScriptEditorPageFactory_H
