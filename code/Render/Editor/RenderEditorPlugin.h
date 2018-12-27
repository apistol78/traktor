/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_RenderEditorPlugin_H
#define traktor_render_RenderEditorPlugin_H

#include "Core/Ref.h"
#include "Editor/IEditorPlugin.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace render
	{

class ShaderDependencyTracker;

/*! \brief Render editor plugin.
 * \ingroup Render
 */
class RenderEditorPlugin : public editor::IEditorPlugin
{
	T_RTTI_CLASS;

public:
	RenderEditorPlugin(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, editor::IEditorPageSite* site) override final;

	virtual void destroy() override final;

	virtual bool handleCommand(const ui::Command& command, bool result) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

	virtual void handleWorkspaceOpened() override final;

	virtual void handleWorkspaceClosed() override final;

private:
	editor::IEditor* m_editor;
	Ref< ShaderDependencyTracker > m_tracker;
};

	}
}

#endif	// traktor_render_RenderEditorPlugin_H
