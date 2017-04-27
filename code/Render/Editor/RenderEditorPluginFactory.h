/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_RenderEditorPluginFactory_H
#define traktor_render_RenderEditorPluginFactory_H

#include "Editor/IEditorPluginFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \brief Render editor plugin factory.
 * \ingroup Render
 */
class T_DLLCLASS RenderEditorPluginFactory : public editor::IEditorPluginFactory
{
	T_RTTI_CLASS;

public:
	virtual void getCommands(std::list< ui::Command >& outCommands) const T_OVERRIDE T_FINAL;

	virtual Ref< editor::IEditorPlugin > createEditorPlugin(editor::IEditor* editor) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_render_RenderEditorPluginFactory_H
