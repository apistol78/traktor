#pragma once

#include "Core/Ref.h"
#include "Editor/IEditorPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IDocument;
class IEditor;
class IEditorPageSite;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace ui
	{

class EdgeConnectEvent;
class EdgeDisconnectEvent;
class GraphControl;
class MouseButtonDownEvent;
class Menu;
class NodeActivateEvent;
class NodeMovedEvent;
class SelectEvent;
class SelectionChangeEvent;
class ToolBar;
class ToolBarButtonClickEvent;

	}

	namespace sound
	{

class AudioChannel;
class AudioSystem;
class GraphAsset;

class T_DLLCLASS GraphEditor : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	explicit GraphEditor(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< GraphAsset > m_graphAsset;
	Ref< ui::ToolBar > m_toolBarGraph;
	Ref< ui::GraphControl > m_graph;
	Ref< ui::Menu > m_menuPopup;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< AudioSystem > m_audioSystem;
	Ref< AudioChannel > m_audioChannel;

	void updateView();

	void eventToolBarGraphClick(ui::ToolBarButtonClickEvent* event);

	void eventButtonDown(ui::MouseButtonDownEvent* event);

	void eventNodeSelect(ui::SelectEvent* event);

	void eventNodeMoved(ui::NodeMovedEvent* event);

	void eventNodeActivated(ui::NodeActivateEvent* event);

	void eventEdgeConnected(ui::EdgeConnectEvent* event);

	void eventEdgeDisconnected(ui::EdgeDisconnectEvent* event);
};

	}
}
