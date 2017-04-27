/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_script_ScriptEditorPluginFactory_H
#define traktor_script_ScriptEditorPluginFactory_H

#include "Editor/IEditorPluginFactory.h"

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
	
/*! \brief
 * \ingroup Script
 */
class T_DLLCLASS ScriptEditorPluginFactory : public editor::IEditorPluginFactory
{
	T_RTTI_CLASS;

public:
	virtual void getCommands(std::list< ui::Command >& outCommands) const T_OVERRIDE T_FINAL;

	virtual Ref< editor::IEditorPlugin > createEditorPlugin(editor::IEditor* editor) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_script_ScriptEditorPluginFactory_H
