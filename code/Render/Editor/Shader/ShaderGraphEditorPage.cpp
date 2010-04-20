#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/Settings.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Editor/UndoStack.h"
#include "Editor/IBrowseFilter.h"
#include "I18N/Text.h"
#include "Render/Editor/TextureAsset.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Shader/Edge.h"
#include "Render/Editor/Shader/NodeCategories.h"
#include "Render/Editor/Shader/NodeFacade.h"
#include "Render/Editor/Shader/ShaderGraphEditorClipboardData.h"
#include "Render/Editor/Shader/ShaderGraphEditorPage.h"
#include "Render/Editor/Shader/ShaderGraphStatic.h"
#include "Render/Editor/Shader/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/ShaderGraphValidator.h"
#include "Render/Editor/Shader/QuickMenuTool.h"
#include "Render/Editor/Shader/Facades/DefaultNodeFacade.h"
#include "Render/Editor/Shader/Facades/ColorNodeFacade.h"
#include "Render/Editor/Shader/Facades/CommentNodeFacade.h"
#include "Render/Editor/Shader/Facades/InterpolatorNodeFacade.h"
#include "Render/Editor/Shader/Facades/SwitchNodeFacade.h"
#include "Render/Editor/Shader/Facades/SwizzleNodeFacade.h"
#include "Render/Editor/Shader/Facades/ExternalNodeFacade.h"
#include "Render/Editor/Shader/Facades/TextureNodeFacade.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Command.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/PopupMenu.h"
#include "Ui/MenuItem.h"
#include "Ui/MessageBox.h"
#include "Ui/TableLayout.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/Graph/GraphControl.h"
#include "Ui/Custom/Graph/PaintSettings.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/Edge.h"
#include "Ui/Custom/Graph/Pin.h"

// Resources
#include "Resources/Alignment.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class RefereeFilter : public editor::IBrowseFilter
{
public:
	RefereeFilter(const Guid& fragmentGuid)
	:	m_fragmentGuid(fragmentGuid)
	{
	}

	virtual bool acceptable(db::Instance* instance) const
	{
		Ref< ShaderGraph > shaderGraph = instance->getObject< ShaderGraph >();
		if (!shaderGraph)
			return false;

		RefArray< External > externalNodes;
		if (!shaderGraph->findNodesOf< External >(externalNodes))
			return false;

		for (RefArray< External >::iterator i = externalNodes.begin(); i != externalNodes.end(); ++i)
		{
			if ((*i)->getFragmentGuid() == m_fragmentGuid)
				return true;
		}

		return false;
	}

private:
	Guid m_fragmentGuid;
};

struct RemoveInputPortPred
{
	bool m_connectable;
	bool m_optional;

	RemoveInputPortPred(bool connectable, bool optional)
	:	m_connectable(connectable)
	,	m_optional(optional)
	{
	}

	bool operator () (InputPort* ip) const
	{
		return ip->isConnectable() == m_connectable && ip->isOptional() == m_optional;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphEditorPage", ShaderGraphEditorPage, editor::IEditorPage)

ShaderGraphEditorPage::ShaderGraphEditorPage(editor::IEditor* editor)
:	m_editor(editor)
,	m_lastValidationResult(true)
{
}

bool ShaderGraphEditorPage::create(ui::Container* parent, editor::IEditorPageSite* site)
{
	m_site = site;
	T_ASSERT (m_site);

	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	// Create our custom toolbar.
	m_toolBar = new ui::custom::ToolBar();
	m_toolBar->create(container);
	m_toolBar->addImage(ui::Bitmap::load(c_ResourceAlignment, sizeof(c_ResourceAlignment), L"png"), 11);
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SHADERGRAPH_OPEN_REFEREE"), ui::Command(L"ShaderGraph.Editor.OpenReferee"), 6));
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SHADERGRAPH_CENTER"), ui::Command(L"ShaderGraph.Editor.Center"), 7));
	m_toolBar->addItem(new ui::custom::ToolBarSeparator());
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SHADERGRAPH_ALIGN_LEFT"), ui::Command(L"ShaderGraph.Editor.AlignLeft"), 0));
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SHADERGRAPH_ALIGN_RIGHT"), ui::Command(L"ShaderGraph.Editor.AlignRight"), 1));
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SHADERGRAPH_ALIGN_TOP"), ui::Command(L"ShaderGraph.Editor.AlignTop"), 2));
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SHADERGRAPH_ALIGN_BOTTOM"), ui::Command(L"ShaderGraph.Editor.AlignBottom"), 3));
	m_toolBar->addItem(new ui::custom::ToolBarSeparator());
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SHADERGRAPH_EVEN_VERTICALLY"), ui::Command(L"ShaderGraph.Editor.EvenSpaceVertically"), 4));
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SHADERGRAPH_EVEN_HORIZONTALLY"), ui::Command(L"ShaderGraph.Editor.EventSpaceHorizontally"), 5));
	m_toolBar->addItem(new ui::custom::ToolBarSeparator());
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SHADERGRAPH_REMOVE_UNUSED_NODES"), ui::Command(L"ShaderGraph.Editor.RemoveUnusedNodes"), 8));
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SHADERGRAPH_AUTO_MERGE_BRANCHES"), ui::Command(L"ShaderGraph.Editor.AutoMergeBranches"), 9));
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SHADERGRAPH_UPDATE_FRAGMENTS"), ui::Command(L"ShaderGraph.Editor.UpdateFragments"), 10));
	m_toolBar->addClickEventHandler(ui::createMethodHandler(this, &ShaderGraphEditorPage::eventToolClick));

	// Create shader graph editor control.
	m_editorGraph = new ui::custom::GraphControl();
	m_editorGraph->create(container);
	m_editorGraph->addButtonDownEventHandler(ui::createMethodHandler(this, &ShaderGraphEditorPage::eventButtonDown));
	m_editorGraph->addSelectEventHandler(ui::createMethodHandler(this, &ShaderGraphEditorPage::eventSelect));
	m_editorGraph->addNodeMovedEventHandler(ui::createMethodHandler(this, &ShaderGraphEditorPage::eventNodeMoved));
	m_editorGraph->addNodeDoubleClickEventHandler(ui::createMethodHandler(this, &ShaderGraphEditorPage::eventNodeDoubleClick));
	m_editorGraph->addEdgeConnectEventHandler(ui::createMethodHandler(this, &ShaderGraphEditorPage::eventEdgeConnect));
	m_editorGraph->addEdgeDisconnectEventHandler(ui::createMethodHandler(this, &ShaderGraphEditorPage::eventEdgeDisconnect));

	// Modify graph control settings.
	Ref< ui::custom::PaintSettings > paintSettings = m_editorGraph->getPaintSettings();
	paintSettings->setSmoothSpline(m_editor->getSettings()->getProperty< PropertyBoolean >(L"ShaderEditor.SmoothSpline"));

	// Build popup menu.
	m_menuPopup = new ui::PopupMenu();
	m_menuPopup->create();
	Ref< ui::MenuItem > menuItemCreate = new ui::MenuItem(i18n::Text(L"SHADERGRAPH_CREATE_NODE"));
	
	std::map< std::wstring, Ref< ui::MenuItem > > categories;
	for (size_t i = 0; i < sizeof_array(c_nodeCateogories); ++i)
	{
		if (categories.find(c_nodeCateogories[i].category) == categories.end())
		{
			categories[c_nodeCateogories[i].category] = new ui::MenuItem(i18n::Text(c_nodeCateogories[i].category));
			menuItemCreate->add(
				categories[c_nodeCateogories[i].category]
			);
		}

		std::wstring title = c_nodeCateogories[i].type.getName();
		size_t p = title.find_last_of(L'.');
		if (p > 0)
			title = i18n::Text(L"SHADERGRAPH_NODE_" + toUpper(title.substr(p + 1)));

		categories[c_nodeCateogories[i].category]->add(
			new ui::MenuItem(ui::Command(i, L"ShaderGraph.Editor.Create"), title)
		);
	}

	m_menuPopup->add(menuItemCreate);
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"SHADERGRAPH_DELETE_NODE")));

	// Build quick menu.
	m_menuQuick = new QuickMenuTool();
	m_menuQuick->create(m_editorGraph);

	// Setup node facades.
	std::vector< const TypeInfo* > nodeTypes;
	type_of< Node >().findAllOf(nodeTypes);

	for (std::vector< const TypeInfo* >::const_iterator i = nodeTypes.begin(); i != nodeTypes.end(); ++i)
		m_nodeFacades[*i] = new DefaultNodeFacade(m_editorGraph);

	m_nodeFacades[&type_of< Color >()] = new ColorNodeFacade(m_editorGraph);
	m_nodeFacades[&type_of< Comment >()] = new CommentNodeFacade(m_editorGraph);
	m_nodeFacades[&type_of< Interpolator >()] = new InterpolatorNodeFacade();
	m_nodeFacades[&type_of< Switch >()] = new SwitchNodeFacade(m_editorGraph);
	m_nodeFacades[&type_of< Swizzle >()] = new SwizzleNodeFacade(m_editorGraph);
	m_nodeFacades[&type_of< External >()] = new ExternalNodeFacade(m_editorGraph);
	m_nodeFacades[&type_of< Texture >()] = new TextureNodeFacade(m_editorGraph);

	m_undoStack = new editor::UndoStack();
	m_lastValidationResult = true;

	return true;
}

void ShaderGraphEditorPage::destroy()
{
	m_nodeFacades.clear();
	safeDestroy(m_editorGraph);
	safeDestroy(m_menuPopup);
	safeDestroy(m_menuQuick);
}

void ShaderGraphEditorPage::activate()
{
}

void ShaderGraphEditorPage::deactivate()
{
}

bool ShaderGraphEditorPage::setDataObject(db::Instance* instance, Object* data)
{
	m_shaderGraph = dynamic_type_cast< ShaderGraph* >(data);
	if (!m_shaderGraph)
		return false;

	m_editorGraph->removeAllEdges();
	m_editorGraph->removeAllNodes();

	createEditorNodes(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);

	m_editorGraph->center();

	updateGraph();

	m_site->setPropertyObject(0);
	m_undoStack = new editor::UndoStack();

	if (instance)
		m_fragmentGuid = instance->getGuid();

	return true;
}

Ref< db::Instance > ShaderGraphEditorPage::getDataInstance()
{
	return m_shaderGraphInstance;
}

Ref< Object > ShaderGraphEditorPage::getDataObject()
{
	return m_shaderGraph;
}

bool ShaderGraphEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	const TypeInfo* primaryType = instance->getPrimaryType();
	T_ASSERT (primaryType);

	// Create texture node in case of a TextureAsset.
	if (is_type_of< TextureAsset >(*primaryType))
	{
		Ref< Texture > shaderNode = new Texture(instance->getGuid());
		m_shaderGraph->addNode(shaderNode);

		ui::Point absolutePosition = m_editorGraph->screenToClient(position) - m_editorGraph->getOffset();
		shaderNode->setPosition(std::make_pair(absolutePosition.x, absolutePosition.y));

		Ref< ui::custom::Node > editorNode = createEditorNode(shaderNode);
		m_editorGraph->addNode(editorNode);

		updateGraph();
	}
	// Create an external node in case of ShaderGraph.
	else if (is_type_of< ShaderGraph >(*primaryType))
	{
		Ref< ShaderGraph > fragmentGraph = instance->getObject< ShaderGraph >();
		T_ASSERT (fragmentGraph);

		Ref< External > shaderNode = new External(
			instance->getGuid(),
			fragmentGraph
		);
		m_shaderGraph->addNode(shaderNode);

		ui::Point absolutePosition = m_editorGraph->screenToClient(position) - m_editorGraph->getOffset();
		shaderNode->setPosition(std::make_pair(absolutePosition.x, absolutePosition.y));

		Ref< ui::custom::Node > editorNode = createEditorNode(shaderNode);
		m_editorGraph->addNode(editorNode);

		updateGraph();
	}
	else
		return false;

	return true;
}

bool ShaderGraphEditorPage::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.PropertiesChanged")
	{
		// Save undo state.
		m_undoStack->push(m_shaderGraph);

		refreshGraph();
		updateGraph();
	}
	else if (command == L"Editor.Cut" || command == L"Editor.Copy")
	{
		RefArray< ui::custom::Node > selectedNodes;
		if (m_editorGraph->getSelectedNodes(selectedNodes) > 0)
		{
			// Also copy edges which are affected by selected nodes.
			RefArray< ui::custom::Edge > selectedEdges;
			m_editorGraph->getConnectedEdges(selectedNodes, true, selectedEdges);

			Ref< ShaderGraphEditorClipboardData > data = new ShaderGraphEditorClipboardData();
			
			ui::Rect bounds(0, 0, 0, 0);
			for (RefArray< ui::custom::Node >::iterator i = selectedNodes.begin(); i != selectedNodes.end(); ++i)
			{
				Ref< Node > shaderNode = (*i)->getData< Node >(L"SHADERNODE");
				T_ASSERT (shaderNode);
				data->addNode(shaderNode);

				if (i != selectedNodes.begin())
				{
					ui::Rect rc = (*i)->calculateRect();
					bounds.left = std::min(bounds.left, rc.left);
					bounds.top = std::min(bounds.top, rc.top);
					bounds.right = std::max(bounds.right, rc.right);
					bounds.bottom = std::max(bounds.bottom, rc.bottom);
				}
				else
					bounds = (*i)->calculateRect();
			}

			data->setBounds(bounds);

			for (RefArray< ui::custom::Edge >::iterator i = selectedEdges.begin(); i != selectedEdges.end(); ++i)
			{
				Ref< Edge > shaderEdge = (*i)->getData< Edge >(L"SHADEREDGE");
				T_ASSERT (shaderEdge);
				data->addEdge(shaderEdge);
			}

			ui::Application::getInstance()->getClipboard()->setObject(data);

			// Remove edges and nodes from graphs if user cuts.
			if (command == L"Editor.Cut")
			{
				// Save undo state.
				m_undoStack->push(m_shaderGraph);

				// Remove edges which are connected to any selected node, not only those who connects to both selected end nodes.
				selectedEdges.resize(0);
				m_editorGraph->getConnectedEdges(selectedNodes, false, selectedEdges);

				for (RefArray< ui::custom::Edge >::iterator i = selectedEdges.begin(); i != selectedEdges.end(); ++i)
				{
					m_shaderGraph->removeEdge((*i)->getData< Edge >(L"SHADEREDGE"));
					m_editorGraph->removeEdge(*i);
				}

				for (RefArray< ui::custom::Node >::iterator i = selectedNodes.begin(); i != selectedNodes.end(); ++i)
				{
					m_shaderGraph->removeNode((*i)->getData< Node >(L"SHADERNODE"));
					m_editorGraph->removeNode(*i);
				}
			}
		}
	}
	else if (command == L"Editor.Paste")
	{
		Ref< ShaderGraphEditorClipboardData > data = dynamic_type_cast< ShaderGraphEditorClipboardData* >(
			ui::Application::getInstance()->getClipboard()->getObject()
		);
		if (data)
		{
			// Save undo state.
			m_undoStack->push(m_shaderGraph);

			const ui::Rect& bounds = data->getBounds();

			ui::Size graphSize = m_editorGraph->getInnerRect().getSize();
			int centerLeft = (graphSize.cx - bounds.getWidth()) / 2 - m_editorGraph->getOffset().cx;
			int centerTop = (graphSize.cy - bounds.getHeight()) / 2 - m_editorGraph->getOffset().cy;

			for (RefArray< Node >::const_iterator i = data->getNodes().begin(); i != data->getNodes().end(); ++i)
			{
				std::pair< int, int > position = (*i)->getPosition();
				position.first = (position.first - bounds.left) + centerLeft;
				position.second = (position.second - bounds.top) + centerTop;
				(*i)->setPosition(position);

				// Add node to graph.
				m_shaderGraph->addNode(*i);
			}

			for (RefArray< Edge >::const_iterator i = data->getEdges().begin(); i != data->getEdges().end(); ++i)
				m_shaderGraph->addEdge(*i);

			createEditorNodes(
				data->getNodes(),
				data->getEdges()
			);
			updateGraph();
		}
	}
	else if (command == L"Editor.SelectAll")
	{
		m_editorGraph->selectAllNodes();
		updateGraph();
	}
	else if (command == L"Editor.Delete")
	{
		RefArray< ui::custom::Node > nodes;
		if (m_editorGraph->getSelectedNodes(nodes) <= 0)
			return false;

		// Save undo state.
		m_undoStack->push(m_shaderGraph);

		// Remove edges first which are connected to selected nodes.
		RefArray< ui::custom::Edge > edges;
		m_editorGraph->getConnectedEdges(nodes, false, edges);

		for (RefArray< ui::custom::Edge >::iterator i = edges.begin(); i != edges.end(); ++i)
		{
			ui::custom::Edge* editorEdge = *i;
			Ref< Edge > shaderEdge = editorEdge->getData< Edge >(L"SHADEREDGE");

			m_editorGraph->removeEdge(editorEdge);
			m_shaderGraph->removeEdge(shaderEdge);
		}

		for (RefArray< ui::custom::Node >::iterator i = nodes.begin(); i != nodes.end(); ++i)
		{
			ui::custom::Node* editorNode = *i;
			Ref< Node > shaderNode = editorNode->getData< Node >(L"SHADERNODE");

			m_editorGraph->removeNode(editorNode);
			m_shaderGraph->removeNode(shaderNode);
		}

		updateGraph();
	}
	else if (command == L"Editor.Undo")
	{
		// Restore last saved step.
		if (m_undoStack->canUndo())
		{
			Ref< ShaderGraph > shaderGraph = dynamic_type_cast< ShaderGraph* >(m_undoStack->undo(m_shaderGraph));
			T_ASSERT (shaderGraph);

			m_shaderGraph = shaderGraph;

			m_editorGraph->removeAllEdges();
			m_editorGraph->removeAllNodes();

			createEditorNodes(
				m_shaderGraph->getNodes(),
				m_shaderGraph->getEdges()
			);

			updateGraph();

			m_site->setPropertyObject(0);
		}
	}
	else if (command == L"Editor.Redo")
	{
		// Redo last undone step.
		if (m_undoStack->canRedo())
		{
			Ref< ShaderGraph > shaderGraph = dynamic_type_cast< ShaderGraph* >(m_undoStack->redo(m_shaderGraph));
			T_ASSERT (shaderGraph);

			m_shaderGraph = shaderGraph;

			m_editorGraph->removeAllEdges();
			m_editorGraph->removeAllNodes();

			createEditorNodes(
				m_shaderGraph->getNodes(),
				m_shaderGraph->getEdges()
			);

			updateGraph();

			m_site->setPropertyObject(0);
		}
	}
	else if (command == L"ShaderGraph.Editor.OpenReferee")
	{
		RefereeFilter filter(m_fragmentGuid);
		Ref< db::Instance > refereeInstance = m_editor->browseInstance(&filter);
		if (refereeInstance)
			m_editor->openEditor(refereeInstance);
	}
	else if (command == L"ShaderGraph.Editor.Center")
	{
		m_editorGraph->center();
	}
	else if (command == L"ShaderGraph.Editor.AlignLeft")
	{
		// Save undo state.
		m_undoStack->push(m_shaderGraph);
		m_editorGraph->alignNodes(ui::custom::GraphControl::AnLeft);
	}
	else if (command == L"ShaderGraph.Editor.AlignRight")
	{
		// Save undo state.
		m_undoStack->push(m_shaderGraph);
		m_editorGraph->alignNodes(ui::custom::GraphControl::AnRight);
	}
	else if (command == L"ShaderGraph.Editor.AlignTop")
	{
		// Save undo state.
		m_undoStack->push(m_shaderGraph);
		m_editorGraph->alignNodes(ui::custom::GraphControl::AnTop);
	}
	else if (command == L"ShaderGraph.Editor.AlignBottom")
	{
		// Save undo state.
		m_undoStack->push(m_shaderGraph);
		m_editorGraph->alignNodes(ui::custom::GraphControl::AnBottom);
	}
	else if (command == L"ShaderGraph.Editor.EvenSpaceVertically")
	{
		// Save undo state.
		m_undoStack->push(m_shaderGraph);
		m_editorGraph->evenSpace(ui::custom::GraphControl::EsVertically);
	}
	else if (command == L"ShaderGraph.Editor.EventSpaceHorizontally")
	{
		// Save undo state.
		m_undoStack->push(m_shaderGraph);
		m_editorGraph->evenSpace(ui::custom::GraphControl::EsHorizontally);
	}
	else if (command == L"ShaderGraph.Editor.RemoveUnusedNodes")
	{
		// Save undo state.
		m_undoStack->push(m_shaderGraph);

		m_shaderGraph = ShaderGraphOptimizer(m_shaderGraph).removeUnusedBranches();

		m_editorGraph->removeAllEdges();
		m_editorGraph->removeAllNodes();

		createEditorNodes(
			m_shaderGraph->getNodes(),
			m_shaderGraph->getEdges()
		);

		updateGraph();

		m_site->setPropertyObject(0);
	}
	else if (command == L"ShaderGraph.Editor.AutoMergeBranches")
	{
		// Save undo state.
		m_undoStack->push(m_shaderGraph);

		m_shaderGraph = ShaderGraphOptimizer(m_shaderGraph).mergeBranches();

		m_editorGraph->removeAllEdges();
		m_editorGraph->removeAllNodes();

		createEditorNodes(
			m_shaderGraph->getNodes(),
			m_shaderGraph->getEdges()
		);

		updateGraph();

		m_site->setPropertyObject(0);
	}
	else if (command == L"ShaderGraph.Editor.UpdateFragments")
	{
		RefArray< ui::custom::Node > selectedNodes;
		m_editorGraph->getSelectedNodes(selectedNodes);

		// Get selected external nodes; ie fragments.
		RefArray< External > selectedExternals;
		for (RefArray< ui::custom::Node >::const_iterator i = selectedNodes.begin(); i != selectedNodes.end(); ++i)
		{
			Ref< External > selectedExternal = (*i)->getData< External >(L"SHADERNODE");
			if (selectedExternal)
				selectedExternals.push_back(selectedExternal);
		}

		if (!selectedExternals.empty())
		{
			// Save undo state.
			m_undoStack->push(m_shaderGraph);

			for (RefArray< External >::const_iterator i = selectedExternals.begin(); i != selectedExternals.end(); ++i)
				updateExternalNode(*i);

			m_editorGraph->removeAllEdges();
			m_editorGraph->removeAllNodes();

			createEditorNodes(
				m_shaderGraph->getNodes(),
				m_shaderGraph->getEdges()
			);

			updateGraph();

			m_site->setPropertyObject(0);
		}
	}
	else if (command == L"ShaderGraph.Editor.QuickMenu")
	{
		const TypeInfo* typeInfo = m_menuQuick->showMenu();
		if (typeInfo)
		{
			m_undoStack->push(m_shaderGraph);
			createNode(
				typeInfo,
				m_editorGraph->getInnerRect().getCenter() - m_editorGraph->getOffset()
			);
		}
		m_editorGraph->setFocus();
	}
	else
		return false;

	m_editorGraph->update();

	return true;
}

void ShaderGraphEditorPage::handleDatabaseEvent(const Guid& eventId)
{
}

void ShaderGraphEditorPage::createEditorNodes(const RefArray< Node >& shaderNodes, const RefArray< Edge >& shaderEdges)
{
	// Keep a map from shader nodes to editor nodes.
	std::map< Ref< Node >, Ref< ui::custom::Node > > nodeMap;

	// Create editor nodes for each shader node.
	for (RefArray< Node >::const_iterator i = shaderNodes.begin(); i != shaderNodes.end(); ++i)
	{
		Node* shaderNode = *i;
		Ref< ui::custom::Node > editorNode = createEditorNode(shaderNode);
		m_editorGraph->addNode(editorNode);
		nodeMap[shaderNode] = editorNode;
	}

	// Create editor edges for each shader edge.
	for (RefArray< Edge >::const_iterator i = shaderEdges.begin(); i != shaderEdges.end(); ++i)
	{
		Edge* shaderEdge = *i;

		const OutputPin* shaderSourcePin = shaderEdge->getSource();
		if (!shaderSourcePin)
		{
			log::warning << L"Invalid shader edge, no source pin" << Endl;
			continue;
		}

		const InputPin* shaderDestinationPin = shaderEdge->getDestination();
		if (!shaderDestinationPin)
		{
			log::warning << L"Invalid shader edge, no destination pin" << Endl;
			continue;
		}

		Ref< ui::custom::Node > editorSourceNode = nodeMap[shaderSourcePin->getNode()];
		if (!editorSourceNode)
		{
			log::warning << L"Invalid shader pin, no editor source node found of pin \"" << shaderSourcePin->getName() << L"\"" << Endl;
			continue;
		}

		Ref< ui::custom::Node > editorDestinationNode = nodeMap[shaderDestinationPin->getNode()];
		if (!editorDestinationNode)
		{
			log::warning << L"Invalid shader pin, no editor destination node found of pin \"" << shaderDestinationPin->getName() << L"\"" << Endl;
			continue;
		}

		Ref< ui::custom::Pin > editorSourcePin = editorSourceNode->findOutputPin(shaderSourcePin->getName());
		if (!editorSourcePin)
		{
			log::warning << L"Unable to find editor source pin \"" << shaderSourcePin->getName() << L"\"" << Endl;
			continue;
		}

		Ref< ui::custom::Pin > editorDestinationPin = editorDestinationNode->findInputPin(shaderDestinationPin->getName());
		if (!editorDestinationPin)
		{
			log::warning << L"Unable to find editor destination pin \"" << shaderDestinationPin->getName() << L"\"" << Endl;
			continue;
		}

		Ref< ui::custom::Edge > editorEdge = new ui::custom::Edge(editorSourcePin, editorDestinationPin);
		editorEdge->setData(L"SHADEREDGE", shaderEdge);

		m_editorGraph->addEdge(editorEdge);
	}
}

Ref< ui::custom::Node > ShaderGraphEditorPage::createEditorNode(Node* shaderNode)
{
	Ref< NodeFacade > nodeFacade = m_nodeFacades[&type_of(shaderNode)];
	T_ASSERT_M (nodeFacade, L"No node facade class found");

	Ref< ui::custom::Node > editorNode = nodeFacade->createEditorNode(
		m_editor,
		m_editorGraph,
		shaderNode
	);

	if (!editorNode)
		return 0;

	for (int j = 0; j < shaderNode->getInputPinCount(); ++j)
	{
		const InputPin* inputPin = shaderNode->getInputPin(j);
		editorNode->createInputPin(
			inputPin->getName(),
			!inputPin->isOptional()
		);
	}

	for (int j = 0; j < shaderNode->getOutputPinCount(); ++j)
	{
		const OutputPin* outputPin = shaderNode->getOutputPin(j);
		editorNode->createOutputPin(
			outputPin->getName()
		);
	}

	editorNode->setComment(shaderNode->getComment());

	editorNode->setData(L"SHADERNODE", shaderNode);
	editorNode->setData(L"FACADE", nodeFacade);

	return editorNode;
}

void ShaderGraphEditorPage::createNode(const TypeInfo* nodeType, const ui::Point& at)
{
	Ref< Node > shaderNode = m_nodeFacades[nodeType]->createShaderNode(nodeType, m_editor);
	if (!shaderNode)
		return;

	// Add to shader graph.
	shaderNode->setPosition(std::pair< int, int >(at.x, at.y));
	m_shaderGraph->addNode(shaderNode);

	// Create editor node from shader node.
	Ref< ui::custom::Node > editorNode = createEditorNode(shaderNode);
	m_editorGraph->addNode(editorNode);
	updateGraph();
}

void ShaderGraphEditorPage::refreshGraph()
{
	// Refresh editor nodes.
	RefArray< ui::custom::Node >& editorNodes = m_editorGraph->getNodes();
	for (RefArray< ui::custom::Node >::const_iterator i = editorNodes.begin(); i != editorNodes.end(); ++i)
	{
		ui::custom::Node* editorNode = *i;

		Node* shaderNode = editorNode->getData< Node >(L"SHADERNODE");
		editorNode->setComment(
			shaderNode->getComment()
		);
		
		if (is_a< External >(shaderNode))
		{
			Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(static_cast< External* >(shaderNode)->getFragmentGuid());
			editorNode->setTitle(instance ? instance->getName() : L"{ Null reference }");
			editorNode->setInfo(L"");
		}
		else
			editorNode->setInfo(
				shaderNode->getInformation()
			);

		const std::pair< int, int >& position = shaderNode->getPosition();
		editorNode->setPosition(ui::Point(
			position.first,
			position.second
		));
	}
}

void ShaderGraphEditorPage::updateGraph()
{
	// Validate shader graph.
	std::vector< const Node* > errorNodes;
	bool validationResult = ShaderGraphValidator(m_shaderGraph).validate(ShaderGraphValidator::SgtFragment, &errorNodes);

	// Indicate if all errors has been corrected.
	if (validationResult && !m_lastValidationResult)
		log::info << L"Validation succeeded" << Endl;

	m_lastValidationResult = validationResult;

	// Update validation indication of each node.
	const RefArray< ui::custom::Node >& editorNodes = m_editorGraph->getNodes();
	for (RefArray< ui::custom::Node >::const_iterator i = editorNodes.begin(); i != editorNodes.end(); ++i)
	{
		Ref< Node > shaderNode = (*i)->getData< Node >(L"SHADERNODE");
		T_ASSERT (shaderNode);

		Ref< NodeFacade > nodeFacade = (*i)->getData< NodeFacade >(L"FACADE");
		T_ASSERT (nodeFacade);

		if (std::find(errorNodes.begin(), errorNodes.end(), shaderNode) != errorNodes.end())
			nodeFacade->setValidationIndicator(*i, false);
		else
			nodeFacade->setValidationIndicator(*i, true);
	}

	// Redraw editor graph.
	m_editorGraph->update();
}

void ShaderGraphEditorPage::updateExternalNode(External* external)
{
	// Get fragment graph from source database.
	Ref< ShaderGraph > fragmentGraph = m_editor->getSourceDatabase()->getObjectReadOnly< ShaderGraph >(external->getFragmentGuid());
	if (!fragmentGraph)
	{
		ui::MessageBox::show(
			i18n::Text(L"SHADERGRAPH_ERROR_MISSING_FRAGMENT_MESSAGE"),
			i18n::Text(L"SHADERGRAPH_ERROR_MISSING_FRAGMENT_CAPTION"),
			ui::MbIconError | ui::MbOk
		);
		return;
	}

	// Get input ports; remove non-connectable ports.
	RefArray< InputPort > fragmentInputs;
	fragmentGraph->findNodesOf< InputPort >(fragmentInputs);

	RefArray< InputPort >::iterator
		i = std::remove_if(fragmentInputs.begin(), fragmentInputs.end(), RemoveInputPortPred(false, false)); fragmentInputs.erase(i, fragmentInputs.end());
		i = std::remove_if(fragmentInputs.begin(), fragmentInputs.end(), RemoveInputPortPred(false, true));  fragmentInputs.erase(i, fragmentInputs.end());

	// Get output ports.
	RefArray< OutputPort > fragmentOutputs;
	fragmentGraph->findNodesOf< OutputPort >(fragmentOutputs);

	// Get input-/output pins; these might differ if fragment has been updated.
	uint32_t externalInputPinCount = external->getInputPinCount();
	uint32_t externalOutputPinCount = external->getOutputPinCount();

	std::vector< const InputPin* > externalInputPins(externalInputPinCount);
	for (uint32_t i = 0; i < externalInputPinCount; ++i)
		externalInputPins[i] = external->getInputPin(i);

	std::vector< const OutputPin* > externalOutputPins(externalOutputPinCount);
	for (uint32_t i = 0; i < externalOutputPinCount; ++i)
		externalOutputPins[i] = external->getOutputPin(i);

	// Remove input ports and pins which match.
	for (RefArray< InputPort >::iterator i = fragmentInputs.begin(); i != fragmentInputs.end(); )
	{
		std::vector< const InputPin* >::iterator j = externalInputPins.begin();
		while (j != externalInputPins.end())
		{
			if ((*i)->getName() == (*j)->getName())
				break;
			++j;
		}
		if (j != externalInputPins.end())
		{
			i = fragmentInputs.erase(i);
			externalInputPins.erase(j);
		}
		else
			++i;
	}

	// Remove output ports and pins which match.
	for (RefArray< OutputPort >::iterator i = fragmentOutputs.begin(); i != fragmentOutputs.end(); )
	{
		std::vector< const OutputPin* >::iterator j = externalOutputPins.begin();
		while (j != externalOutputPins.end())
		{
			if ((*i)->getName() == (*j)->getName())
				break;
			++j;
		}
		if (j != externalOutputPins.end())
		{
			i = fragmentOutputs.erase(i);
			externalOutputPins.erase(j);
		}
		else
			++i;
	}

	// If we don't have any ports nor pins there is nothing to update.
	if (
		fragmentInputs.empty() &&
		fragmentOutputs.empty() &&
		externalInputPins.empty() &&
		externalOutputPins.empty()
	)
		return;

	// Remove pins which have their respective ports removed.
	while (!externalInputPins.empty())
	{
		Ref< Edge > edge = m_shaderGraph->findEdge(externalInputPins.back());
		if (edge)
			m_shaderGraph->removeEdge(edge);

		external->removeValue(externalInputPins.back()->getName());
		external->removeInputPin(externalInputPins.back());

		externalInputPins.pop_back();
	}
	while (!externalOutputPins.empty())
	{
		RefSet< Edge > edges;
		m_shaderGraph->findEdges(externalOutputPins.back(), edges);
		for (RefSet< Edge >::const_iterator i = edges.begin(); i != edges.end(); ++i)
			m_shaderGraph->removeEdge(*i);

		external->removeOutputPin(externalOutputPins.back());
		externalOutputPins.pop_back();
	}

	// Add new pins for new ports.
	for (RefArray< InputPort >::iterator i = fragmentInputs.begin(); i != fragmentInputs.end(); ++i)
	{
		external->createInputPin((*i)->getName(), (*i)->isOptional());
		if ((*i)->isOptional())
			external->setValue((*i)->getName(), (*i)->getDefaultValue());
	}
	for (RefArray< OutputPort >::iterator i = fragmentOutputs.begin(); i != fragmentOutputs.end(); ++i)
		external->createOutputPin((*i)->getName());
}

void ShaderGraphEditorPage::eventToolClick(ui::Event* event)
{
	const ui::Command& command = checked_type_cast< ui::CommandEvent* >(event)->getCommand();
	handleCommand(command);
}

void ShaderGraphEditorPage::eventButtonDown(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);
	if (mouseEvent->getButton() != ui::MouseEvent::BtRight)
		return;

	Ref< ui::MenuItem > selected = m_menuPopup->show(m_editorGraph, mouseEvent->getPosition());
	if (!selected)
		return;

	const ui::Command& command = selected->getCommand();

	if (command == L"ShaderGraph.Editor.Create")	// Create
	{
		const TypeInfo& type = c_nodeCateogories[command.getId()].type;

		// Save undo state.
		m_undoStack->push(m_shaderGraph);

		createNode(&type, mouseEvent->getPosition() - m_editorGraph->getOffset());
	}
	else
		handleCommand(command);
}

void ShaderGraphEditorPage::eventSelect(ui::Event* event)
{
	RefArray< ui::custom::Node > nodes;
	if (m_editorGraph->getSelectedNodes(nodes) == 1)
	{
		Ref< Node > shaderNode = nodes[0]->getData< Node >(L"SHADERNODE");
		T_ASSERT (shaderNode);

		m_site->setPropertyObject(shaderNode);
	}
	else
		m_site->setPropertyObject(0);
}

void ShaderGraphEditorPage::eventNodeMoved(ui::Event* event)
{
	Ref< ui::custom::Node > editorNode = checked_type_cast< ui::custom::Node* >(event->getItem());
	T_ASSERT (editorNode);

	// Get shader graph node from editor node.
	Ref< Node > shaderNode = editorNode->getData< Node >(L"SHADERNODE");
	T_ASSERT (shaderNode);

	ui::Point position = editorNode->getPosition();
	if (position.x != shaderNode->getPosition().first || position.y != shaderNode->getPosition().second)
	{
		// Save undo state.
		m_undoStack->push(m_shaderGraph);

		// Reflect position into shader graph node.
		shaderNode->setPosition(std::pair< int, int >(
			editorNode->getPosition().x,
			editorNode->getPosition().y
		));
	}

	// Update properties.
	if (editorNode->isSelected())
		m_site->setPropertyObject(shaderNode);
}

void ShaderGraphEditorPage::eventNodeDoubleClick(ui::Event* event)
{
	Ref< ui::custom::Node > editorNode = checked_type_cast< ui::custom::Node* >(event->getItem());
	Ref< Node > shaderNode = editorNode->getData< Node >(L"SHADERNODE");
	T_ASSERT (shaderNode);

	m_nodeFacades[&type_of(shaderNode)]->editShaderNode(
		m_editor,
		m_editorGraph,
		shaderNode
	);

	// Update properties.
	m_site->setPropertyObject(shaderNode);

	// Refresh graph; information might have changed.
	refreshGraph();
}

void ShaderGraphEditorPage::eventEdgeConnect(ui::Event* event)
{
	Ref< ui::custom::Edge > editorEdge = checked_type_cast< ui::custom::Edge* >(event->getItem());
	Ref< ui::custom::Pin > editorSourcePin = editorEdge->getSourcePin();
	T_ASSERT (editorSourcePin);

	Ref< ui::custom::Pin > editorDestinationPin = editorEdge->getDestinationPin();
	T_ASSERT (editorDestinationPin);

	Ref< Node > shaderSourceNode = editorSourcePin->getNode()->getData< Node >(L"SHADERNODE");
	T_ASSERT (shaderSourceNode);

	Ref< Node > shaderDestinationNode = editorDestinationPin->getNode()->getData< Node >(L"SHADERNODE");
	T_ASSERT (shaderDestinationNode);

	const OutputPin* shaderSourcePin = shaderSourceNode->findOutputPin(editorSourcePin->getName());
	T_ASSERT (shaderSourcePin);

	const InputPin* shaderDestinationPin = shaderDestinationNode->findInputPin(editorDestinationPin->getName());
	T_ASSERT (shaderDestinationPin);

	// Replace existing edge.
	Ref< Edge > shaderEdge = m_shaderGraph->findEdge(shaderDestinationPin);
	if (shaderEdge)
	{
		m_shaderGraph->removeEdge(shaderEdge);

		RefArray< ui::custom::Edge > editorEdges;
		m_editorGraph->getConnectedEdges(editorDestinationPin, editorEdges);
		T_ASSERT (editorEdges.size() == 1);

		m_editorGraph->removeEdge(editorEdges.front());
	}

	// Save undo state.
	m_undoStack->push(m_shaderGraph);

	shaderEdge = new Edge(shaderSourcePin, shaderDestinationPin);
	m_shaderGraph->addEdge(shaderEdge);

	editorEdge->setData(L"SHADEREDGE", shaderEdge);
	m_editorGraph->addEdge(editorEdge);

	updateGraph();
}

void ShaderGraphEditorPage::eventEdgeDisconnect(ui::Event* event)
{
	Ref< ui::custom::Edge > editorEdge = checked_type_cast< ui::custom::Edge* >(event->getItem());
	Ref< Edge > shaderEdge = checked_type_cast< Edge* >(editorEdge->getData(L"SHADEREDGE"));

	// Save undo state.
	m_undoStack->push(m_shaderGraph);

	m_shaderGraph->removeEdge(shaderEdge);

	updateGraph();
}

	}
}
