#pragma once

#include "Editor/IEditorPluginFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_REMOTE_SERVER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace remote
	{

/*! Remote server editor plugin factory.
 * \ingroup Remote
 */
class T_DLLCLASS RemoteEditorPluginFactory : public editor::IEditorPluginFactory
{
	T_RTTI_CLASS;

public:
	virtual void getCommands(std::list< ui::Command >& outCommands) const override final;

	virtual Ref< editor::IEditorPlugin > createEditorPlugin(editor::IEditor* editor) const override final;
};

	}
}

