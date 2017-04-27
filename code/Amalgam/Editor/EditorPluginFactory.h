/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_EditorPluginFactory_H
#define traktor_amalgam_EditorPluginFactory_H

#include "Editor/IEditorPluginFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

/*! \brief Amalgam editor plugin factory.
 * \ingroup Amalgam
 */
class T_DLLCLASS EditorPluginFactory : public editor::IEditorPluginFactory
{
	T_RTTI_CLASS;

public:
	virtual void getCommands(std::list< ui::Command >& outCommands) const T_OVERRIDE T_FINAL;

	virtual Ref< editor::IEditorPlugin > createEditorPlugin(editor::IEditor* editor) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_amalgam_EditorPluginFactory_H
