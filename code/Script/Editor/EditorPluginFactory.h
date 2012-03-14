#ifndef traktor_script_EditorPluginFactory_H
#define traktor_script_EditorPluginFactory_H

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

class T_DLLCLASS EditorPluginFactory : public editor::IEditorPluginFactory
{
	T_RTTI_CLASS;

public:
	virtual void getCommands(std::list< ui::Command >& outCommands) const;

	virtual Ref< editor::IEditorPlugin > createEditorPlugin(editor::IEditor* editor) const;
};

	}
}

#endif	// traktor_script_EditorPluginFactory_H
