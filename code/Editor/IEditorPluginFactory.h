/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_IEditorPluginFactory_H
#define traktor_editor_IEditorPluginFactory_H

#include <list>
#include "Core/Object.h"
#include "Ui/Command.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IEditor;
class IEditorPlugin;

/*! \brief Editor plugin factory.
 * \ingroup Editor
 */
class T_DLLCLASS IEditorPluginFactory : public Object
{
	T_RTTI_CLASS;

public:
	virtual void getCommands(std::list< ui::Command >& outCommands) const = 0;

	virtual Ref< IEditorPlugin > createEditorPlugin(IEditor* editor) const = 0;
};

	}
}

#endif	// traktor_editor_IEditorPluginFactory_H
