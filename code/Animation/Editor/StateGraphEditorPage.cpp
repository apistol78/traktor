#include "Animation/Editor/StateGraphEditorPage.h"
#include "Animation/Animation/StateGraph.h"
#include "Animation/Animation/State.h"
#include "Animation/Animation/Transition.h"
#include "Animation/Animation/Animation.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Database/Instance.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/PopupMenu.h"
#include "Ui/MenuItem.h"
#include "Ui/TableLayout.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/Graph/GraphControl.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/Edge.h"
#include "Ui/Custom/Graph/Pin.h"
#include "Ui/Custom/Graph/DefaultNodeShape.h"
#include "I18N/Text.h"

// Resources
#include "Resources/Alignment.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.StateGraphEditorPage", StateGraphEditorPage, editor::IEditorPage)

StateGraphEditorPage::StateGraphEditorPage(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool StateGraphEditorPage::create(ui::Container* parent, editor::IEditorPageSite* site)
{
	m_site = site;
	T_ASSERT (m_site);

	Ref< ui::Container > container = gc_new< ui::Container >();
	container->create(parent, ui::WsNone, gc_new< ui::TableLayout >(L"100%", L"*,100%", 0, 0));

	// Create our custom toolbar.
	m_toolBar = gc_new< ui::custom::ToolBar >();
	m_toolBar->create(container);
	m_toolBar->addImage(ui::Bitmap::load(c_ResourceAlignment, sizeof(c_ResourceAlignment), L"png"), 7);
	m_toolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"STATEGRAPH_ALIGN_LEFT"), ui::Command(L"StateGraph.Editor.AlignLeft"), 0));
	m_toolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"STATEGRAPH_ALIGN_RIGHT"), ui::Command(L"StateGraph.Editor.AlignRight"), 1));
	m_toolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"STATEGRAPH_ALIGN_TOP"), ui::Command(L"StateGraph.Editor.AlignTop"), 2));
	m_toolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"STATEGRAPH_ALIGN_BOTTOM"), ui::Command(L"StateGraph.Editor.AlignBottom"), 3));
	m_toolBar->addItem(gc_new< ui::custom::ToolBarSeparator >());
	m_toolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"STATEGRAPH_EVEN_VERTICALLY"), ui::Command(L"StateGraph.Editor.EvenSpaceVertically"), 4));
	m_toolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"STATEGRAPH_EVEN_HORIZONTALLY"), ui::Command(L"StateGraph.Editor.EventSpaceHorizontally"), 5));
	m_toolBar->addClickEventHandler(ui::createMethodHandler(this, &StateGraphEditorPage::eventToolClick));

	// Create state graph editor control.
	m_editorGraph = gc_new< ui::custom::GraphControl >();
	m_editorGraph->create(container, ui::custom::GraphControl::WsEdgeSelectable | ui::WsDoubleBuffer | ui::WsAccelerated);
	m_editorGraph->addButtonDownEventHandler(ui::createMethodHandler(this, &StateGraphEditorPage::eventButtonDown));
	m_editorGraph->addSelectEventHandler(ui::createMethodHandler(this, &StateGraphEditorPage::eventSelect));
	m_editorGraph->addNodeMovedEventHandler(ui::createMethodHandler(this, &StateGraphEditorPage::eventNodeMoved));
	m_editorGraph->addEdgeConnectEventHandler(ui::createMethodHandler(this, &StateGraphEditorPage::eventEdgeConnect));
	m_editorGraph->addEdgeDisconnectEventHandler(ui::createMethodHandler(this, &StateGraphEditorPage::eventEdgeDisconnect));

	// Build popup menu.
	m_menuPopup = gc_new< ui::PopupMenu >();
	m_menuPopup->create();
	m_menuPopup->add(gc_new< ui::MenuItem >(ui::Command(L"StateGraph.Editor.Create"), i18n::Text(L"STATEGRAPH_CREATE_STATE")));
	m_menuPopup->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Delete"), i18n::Text(L"STATEGRAPH_DELETE_STATE")));
	m_menuPopup->add(gc_new< ui::MenuItem >(L"-"));
	m_menuPopup->add(gc_new< ui::MenuItem >(ui::Command(L"StateGraph.Editor.SetRoot"), i18n::Text(L"STATEGRAPH_SET_ROOT")));

	return true;
}

void StateGraphEditorPage::destroy()
{
	m_editorGraph->destroy();
	m_menuPopup->destroy();
}

void StateGraphEditorPage::activate()
{
}

void StateGraphEditorPage::deactivate()
{
}

bool StateGraphEditorPage::setDataObject(db::Instance* instance, Object* data)
{
	m_stateGraphInstance = instance;
	m_stateGraph = dynamic_type_cast< StateGraph* >(data);
	if (!m_stateGraph)
		return false;

	m_editorGraph->removeAllEdges();
	m_editorGraph->removeAllNodes();

	createEditorNodes(
		m_stateGraph->getStates(),
		m_stateGraph->getTransitions()
	);

	m_editorGraph->center();

	updateGraph();

	m_site->setPropertyObject(0);

	return true;
}

db::Instance* StateGraphEditorPage::getDataInstance()
{
	return m_stateGraphInstance;
}

Object* StateGraphEditorPage::getDataObject()
{
	return m_stateGraph;
}

void StateGraphEditorPage::propertiesChanged()
{
	// Refresh editor nodes.
	RefArray< ui::custom::Node >& editorNodes = m_editorGraph->getNodes();
	for (RefArray< ui::custom::Node >::const_iterator i = editorNodes.begin(); i != editorNodes.end(); ++i)
	{
		Ref< ui::custom::Node > node = *i;

		Ref< State > state = node->getData< State >(L"STATE");
		node->setTitle(state->getName());

		const std::pair< int, int >& position = state->getPosition();
		node->setPosition(ui::Point(
			position.first,
			position.second
		));
	}

	updateGraph();
}

bool StateGraphEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	const Type* primaryType = instance->getPrimaryType();
	T_ASSERT (primaryType);

	if (is_type_of< Animation >(*primaryType))
	{
		Ref< State > state = gc_new< State >(instance->getName(), instance->getGuid());

		ui::Point absolutePosition = m_editorGraph->screenToClient(position) - m_editorGraph->getOffset();
		state->setPosition(std::pair< int, int >(absolutePosition.x, absolutePosition.y));

		Ref< ui::custom::Node > node = createEditorNode(state);
		m_editorGraph->addNode(node);
		updateGraph();
	}
	else
		return false;

	return true;
}

bool StateGraphEditorPage::handleCommand(const ui::Command& command)
{
	//if (command == L"Editor.Cut" || command == L"Editor.Copy")
	//{
	//	RefArray< ui::custom::Node > selectedNodes;
	//	if (m_editorGraph->getSelectedNodes(selectedNodes) > 0)
	//	{
	//		// Also copy edges which are affected by selected nodes.
	//		RefArray< ui::custom::Edge > selectedEdges;
	//		m_editorGraph->getSelectedEdges(selectedEdges, true);

	//		Ref< ShaderGraphEditorClipboardData > data = gc_new< ShaderGraphEditorClipboardData >();
	//		
	//		ui::Rect bounds(0, 0, 0, 0);
	//		for (RefArray< ui::custom::Node >::iterator i = selectedNodes.begin(); i != selectedNodes.end(); ++i)
	//		{
	//			Ref< Node > shaderNode = (*i)->getData< Node >(L"SHADERNODE");
	//			T_ASSERT (shaderNode);
	//			data->addNode(shaderNode);

	//			if (i != selectedNodes.begin())
	//			{
	//				ui::Rect rc = (*i)->calculateRect();
	//				bounds.left = std::min(bounds.left, rc.left);
	//				bounds.top = std::min(bounds.top, rc.top);
	//				bounds.right = std::max(bounds.right, rc.right);
	//				bounds.bottom = std::max(bounds.bottom, rc.bottom);
	//			}
	//			else
	//				bounds = (*i)->calculateRect();
	//		}

	//		data->setBounds(bounds);

	//		for (RefArray< ui::custom::Edge >::iterator i = selectedEdges.begin(); i != selectedEdges.end(); ++i)
	//		{
	//			Ref< Edge > shaderEdge = (*i)->getData< Edge >(L"SHADEREDGE");
	//			T_ASSERT (shaderEdge);
	//			data->addEdge(shaderEdge);
	//		}

	//		ui::Application::getInstance().getClipboard()->setObject(data);

	//		// Remove edges and nodes from graphs if user cuts.
	//		if (command == L"Editor.Cut")
	//		{
	//			// Save undo state.
	//			m_undoStack->push(m_shaderGraph);

	//			// Remove edges which are connected to any selected node, not only those who connects to both selected end nodes.
	//			selectedEdges.resize(0);
	//			m_editorGraph->getSelectedEdges(selectedEdges, false);

	//			for (RefArray< ui::custom::Edge >::iterator i = selectedEdges.begin(); i != selectedEdges.end(); ++i)
	//			{
	//				m_shaderGraph->removeEdge((*i)->getData< Edge >(L"SHADEREDGE"));
	//				m_editorGraph->removeEdge(*i);
	//			}

	//			for (RefArray< ui::custom::Node >::iterator i = selectedNodes.begin(); i != selectedNodes.end(); ++i)
	//			{
	//				m_shaderGraph->removeNode((*i)->getData< Node >(L"SHADERNODE"));
	//				m_editorGraph->removeNode(*i);
	//			}
	//		}
	//	}
	//}
	//else if (command == L"Editor.Paste")
	//{
	//	Ref< ShaderGraphEditorClipboardData > data = dynamic_type_cast< ShaderGraphEditorClipboardData* >(
	//		ui::Application::getInstance().getClipboard()->getObject()
	//	);
	//	if (data)
	//	{
	//		// Save undo state.
	//		m_undoStack->push(m_shaderGraph);

	//		const ui::Rect& bounds = data->getBounds();

	//		ui::Size graphSize = m_editorGraph->getInnerRect().getSize();
	//		int centerLeft = (graphSize.cx - bounds.getWidth()) / 2 - m_editorGraph->getOffset().cx;
	//		int centerTop = (graphSize.cy - bounds.getHeight()) / 2 - m_editorGraph->getOffset().cy;

	//		for (RefList< Node >::const_iterator i = data->getNodes().begin(); i != data->getNodes().end(); ++i)
	//		{
	//			std::pair< int, int > position = (*i)->getPosition();
	//			position.first = (position.first - bounds.left) + centerLeft;
	//			position.second = (position.second - bounds.top) + centerTop;
	//			(*i)->setPosition(position);

	//			// Add node to graph.
	//			m_shaderGraph->addNode(*i);
	//		}

	//		for (RefList< Edge >::const_iterator i = data->getEdges().begin(); i != data->getEdges().end(); ++i)
	//			m_shaderGraph->addEdge(*i);

	//		createEditorNodes(
	//			data->getNodes(),
	//			data->getEdges()
	//		);
	//		updateGraph();
	//	}
	//}
	/*else*/ if (command == L"Editor.SelectAll")
	{
		m_editorGraph->selectAllNodes();
		updateGraph();
	}
	else if (command == L"Editor.Delete")
	{
		RefArray< ui::custom::Node > nodes;
		if (m_editorGraph->getSelectedNodes(nodes) <= 0)
			return false;

	//	// Save undo state.
	//	m_undoStack->push(m_shaderGraph);

		// First remove transitions which are connected to selected states.
		RefArray< ui::custom::Edge > edges;
		m_editorGraph->getConnectedEdges(nodes, false, edges);

		for (RefArray< ui::custom::Edge >::iterator i = edges.begin(); i != edges.end(); ++i)
		{
			Ref< ui::custom::Edge > edge = *i;
			Ref< Transition > transition = edge->getData< Transition >(L"TRANSITION");

			m_editorGraph->removeEdge(edge);
			m_stateGraph->removeTransition(transition);
		}

		// Then remove all states.
		for (RefArray< ui::custom::Node >::iterator i = nodes.begin(); i != nodes.end(); ++i)
		{
			Ref< ui::custom::Node > node = *i;
			Ref< State > state = node->getData< State >(L"STATE");

			m_editorGraph->removeNode(node);
			m_stateGraph->removeState(state);
		}

		updateGraph();
	}
	//else if (command == L"Editor.Undo")
	//{
	//	// Restore last saved step.
	//	if (m_undoStack->canUndo())
	//	{
	//		Ref< ShaderGraph > shaderGraph = dynamic_type_cast< ShaderGraph* >(m_undoStack->undo(m_shaderGraph));
	//		T_ASSERT (shaderGraph);

	//		m_shaderGraph = shaderGraph;

	//		m_editorGraph->removeAllEdges();
	//		m_editorGraph->removeAllNodes();

	//		createEditorNodes(
	//			m_shaderGraph->getNodes(),
	//			m_shaderGraph->getEdges()
	//		);

	//		updateGraph();

	//		m_editor->setPropertyObject(0);
	//	}
	//}
	//else if (command == L"Editor.Redo")
	//{
	//	// Redo last undone step.
	//	if (m_undoStack->canRedo())
	//	{
	//		Ref< ShaderGraph > shaderGraph = dynamic_type_cast< ShaderGraph* >(m_undoStack->redo(m_shaderGraph));
	//		T_ASSERT (shaderGraph);

	//		m_shaderGraph = shaderGraph;

	//		m_editorGraph->removeAllEdges();
	//		m_editorGraph->removeAllNodes();

	//		createEditorNodes(
	//			m_shaderGraph->getNodes(),
	//			m_shaderGraph->getEdges()
	//		);

	//		updateGraph();

	//		m_editor->setPropertyObject(0);
	//	}
	//}
	else if (command == L"StateGraph.Editor.SetRoot")
	{
		RefArray< ui::custom::Node > selectedNodes;
		if (m_editorGraph->getSelectedNodes(selectedNodes) == 1)
		{
			Ref< State > state = selectedNodes.front()->getData< State >(L"STATE");
			T_ASSERT (state);

			m_stateGraph->setRootState(state);

			// Update color to show which node is root.
			const RefArray< ui::custom::Node >& nodes = m_editorGraph->getNodes();
			for (RefArray< ui::custom::Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
				(*i)->setColor(*i == selectedNodes.front() ? Color(128, 255, 128) : Color(255, 255, 255));
		}
	}
	else if (command == L"StateGraph.Editor.AlignLeft")
	{
		//// Save undo state.
		//m_undoStack->push(m_shaderGraph);
		m_editorGraph->alignNodes(ui::custom::GraphControl::AnLeft);
	}
	else if (command == L"StateGraph.Editor.AlignRight")
	{
		//// Save undo state.
		//m_undoStack->push(m_shaderGraph);
		m_editorGraph->alignNodes(ui::custom::GraphControl::AnRight);
	}
	else if (command == L"StateGraph.Editor.AlignTop")
	{
		//// Save undo state.
		//m_undoStack->push(m_shaderGraph);
		m_editorGraph->alignNodes(ui::custom::GraphControl::AnTop);
	}
	else if (command == L"StateGraph.Editor.AlignBottom")
	{
		//// Save undo state.
		//m_undoStack->push(m_shaderGraph);
		m_editorGraph->alignNodes(ui::custom::GraphControl::AnBottom);
	}
	else if (command == L"StateGraph.Editor.EvenSpaceVertically")
	{
		//// Save undo state.
		//m_undoStack->push(m_shaderGraph);
		m_editorGraph->evenSpace(ui::custom::GraphControl::EsVertically);
	}
	else if (command == L"StateGraph.Editor.EventSpaceHorizontally")
	{
		//// Save undo state.
		//m_undoStack->push(m_shaderGraph);
		m_editorGraph->evenSpace(ui::custom::GraphControl::EsHorizontally);
	}
	else
		return false;

	m_editorGraph->update();

	return true;
}

void StateGraphEditorPage::handleDatabaseEvent(const Guid& eventId)
{
}

void StateGraphEditorPage::createEditorNodes(const RefArray< State >& states, const RefArray< Transition >& transitions)
{
	std::map< Ref< State >, Ref< ui::custom::Node > > nodeMap;

	// Create editor nodes for each state.
	for (RefArray< State >::const_iterator i = states.begin(); i != states.end(); ++i)
	{
		Ref< State > state = *i;
		Ref< ui::custom::Node > node = createEditorNode(state);
		m_editorGraph->addNode(node);
		nodeMap[state] = node;
	}

	// Create editor edges for each transition.
	for (RefArray< Transition >::const_iterator i = transitions.begin(); i != transitions.end(); ++i)
	{
		Ref< Transition > transition = *i;

		Ref< State > from = transition->from();
		Ref< State > to = transition->to();

		Ref< ui::custom::Node > fromNode = nodeMap[from];
		Ref< ui::custom::Node > toNode = nodeMap[to];

		if (!fromNode || !toNode)
			continue;

		Ref< ui::custom::Pin > fromPin = fromNode->findOutputPin(L"Leave");
		T_ASSERT (fromPin);

		Ref< ui::custom::Pin > toPin = toNode->findInputPin(L"Enter");
		T_ASSERT (toPin);

		Ref< ui::custom::Edge > transitionEdge = gc_new< ui::custom::Edge >(fromPin, toPin);
		transitionEdge->setData(L"TRANSITION", transition);

		m_editorGraph->addEdge(transitionEdge);
	}
}

ui::custom::Node* StateGraphEditorPage::createEditorNode(State* state)
{
	Ref< ui::custom::NodeShape > shape = gc_new< ui::custom::DefaultNodeShape >(m_editorGraph);

	Ref< ui::custom::Node > node = gc_new< ui::custom::Node >(
		state->getName(),
		L"",
		ui::Point(
			state->getPosition().first,
			state->getPosition().second
		),
		shape
	);
	node->setColor(m_stateGraph->getRootState() == state ? Color(128, 255, 128) : Color(255, 255, 255));
	node->setData(L"STATE", state);

	Ref< ui::custom::Pin > enterPin = gc_new< ui::custom::Pin >(node, L"Enter", ui::custom::Pin::DrInput, true);
	node->addInputPin(enterPin);

	Ref< ui::custom::Pin > leavePin = gc_new< ui::custom::Pin >(node, L"Leave", ui::custom::Pin::DrOutput, true);
	node->addOutputPin(leavePin);

	return node;
}

void StateGraphEditorPage::createState(const ui::Point& at)
{
	Ref< State > state = gc_new< State >(i18n::Text(L"STATEGRAPH_UNNAMED"), Guid());
	state->setPosition(std::pair< int, int >(at.x, at.y));
	m_stateGraph->addState(state);

	Ref< ui::custom::Node > node = createEditorNode(state);
	m_editorGraph->addNode(node);
	updateGraph();
}

void StateGraphEditorPage::updateGraph()
{
	m_editorGraph->update();
}

void StateGraphEditorPage::eventToolClick(ui::Event* event)
{
	const ui::Command& command = checked_type_cast< ui::CommandEvent* >(event)->getCommand();
	handleCommand(command);
}

void StateGraphEditorPage::eventButtonDown(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);
	if (mouseEvent->getButton() != ui::MouseEvent::BtRight)
		return;

	Ref< ui::MenuItem > selected = m_menuPopup->show(m_editorGraph, mouseEvent->getPosition());
	if (!selected)
		return;

	const ui::Command& command = selected->getCommand();

	if (command == L"StateGraph.Editor.Create")
	{
		createState(mouseEvent->getPosition() - m_editorGraph->getOffset());
	}
	else
		handleCommand(command);
}

void StateGraphEditorPage::eventSelect(ui::Event* event)
{
	RefArray< ui::custom::Node > nodes;
	RefArray< ui::custom::Edge > edges;

	if (m_editorGraph->getSelectedNodes(nodes) == 1)
	{
		Ref< State > state = nodes[0]->getData< State >(L"STATE");
		T_ASSERT (state);

		m_site->setPropertyObject(state);
	}
	else if (m_editorGraph->getSelectedEdges(edges) == 1)
	{
		Ref< Transition > transition = edges[0]->getData< Transition >(L"TRANSITION");
		T_ASSERT (transition);

		m_site->setPropertyObject(transition);
	}
	else
		m_site->setPropertyObject(0);
}

void StateGraphEditorPage::eventNodeMoved(ui::Event* event)
{
	Ref< ui::custom::Node > node = checked_type_cast< ui::custom::Node* >(event->getItem());
	T_ASSERT (node);

	// Get state from editor node.
	Ref< State > state = node->getData< State >(L"STATE");
	T_ASSERT (state);

	ui::Point position = node->getPosition();
	if (position.x != state->getPosition().first || position.y != state->getPosition().second)
	{
		state->setPosition(std::pair< int, int >(
			node->getPosition().x,
			node->getPosition().y
		));
	}

	// Update properties.
	if (node->isSelected())
		m_site->setPropertyObject(state);
}

void StateGraphEditorPage::eventEdgeConnect(ui::Event* event)
{
	Ref< ui::custom::Edge > edge = checked_type_cast< ui::custom::Edge* >(event->getItem());

	Ref< ui::custom::Pin > leavePin = edge->getSourcePin();
	T_ASSERT (leavePin);

	Ref< ui::custom::Pin > enterPin = edge->getDestinationPin();
	T_ASSERT (enterPin);

	Ref< State > leaveState = leavePin->getNode()->getData< State >(L"STATE");
	T_ASSERT (leaveState);

	Ref< State > enterState = enterPin->getNode()->getData< State >(L"STATE");
	T_ASSERT (enterState);

	Ref< Transition > transition = gc_new< Transition >(leaveState, enterState);
	m_stateGraph->addTransition(transition);

	edge->setData(L"TRANSITION", transition);
	m_editorGraph->addEdge(edge);

	updateGraph();
}

void StateGraphEditorPage::eventEdgeDisconnect(ui::Event* event)
{
	Ref< ui::custom::Edge > edge = checked_type_cast< ui::custom::Edge* >(event->getItem());

	Ref< Transition > transition = checked_type_cast< Transition* >(edge->getData(L"TRANSITION"));
	m_stateGraph->removeTransition(transition);

	updateGraph();
}

	}
}
