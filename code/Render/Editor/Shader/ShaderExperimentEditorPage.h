/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Editor/IEditorPage.h"
#include "Resource/Proxy.h"
#include "Ui/Events/AllEvents.h"
#include "Ui/Point.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::editor
{

class IDocument;
class IEditor;
class IEditorPageSite;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::ui
{

class Container;
class EdgeConnectEvent;
class EdgeDisconnectEvent;
class GraphControl;
class Group;
class GroupMovedEvent;
class GridItemContentChangeEvent;
class GridRowDoubleClickEvent;
class GridView;
class Menu;
class Node;
class NodeActivateEvent;
class NodeMovedEvent;
class SelectEvent;
class Splitter;
class SyntaxRichEdit;
class ToolBar;
class ToolBarButtonClickEvent;
class ToolBarDropDown;

}

namespace traktor::render
{

class IRenderSystem;
class IRenderView;
class Shader;
class ShaderExperiment;

class T_DLLEXPORT ShaderExperimentEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	explicit ShaderExperimentEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;

	Ref< ShaderExperiment > m_experiment;

	Ref< ui::Widget > m_renderWidget;

	Ref< resource::IResourceManager > m_resourceManager;
	Ref< IRenderSystem > m_renderSystem;
	Ref< IRenderView > m_renderView;

	resource::Proxy< Shader > m_shader;

	void executeExperiment();
};

}
