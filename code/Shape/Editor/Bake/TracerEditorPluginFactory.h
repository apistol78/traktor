#pragma once

#include "Editor/IEditorPluginFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace shape
    {

class T_DLLCLASS TracerEditorPluginFactory : public editor::IEditorPluginFactory
{
    T_RTTI_CLASS;

public:
	virtual void getCommands(std::list< ui::Command >& outCommands) const override final;

	virtual Ref< editor::IEditorPlugin > createEditorPlugin(editor::IEditor* editor) const override final;
};

    }
}