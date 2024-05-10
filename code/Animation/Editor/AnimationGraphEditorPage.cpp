/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/Animation.h"
#include "Animation/Animation/AnimationGraph.h"
#include "Animation/Animation/AnimationGraphPoseController.h"
#include "Animation/Animation/AnimationGraphPoseControllerData.h"
#include "Animation/Animation/StateNode.h"
#include "Animation/Animation/Transition.h"
#include "Animation/Editor/AnimationGraphEditorPage.h"
#include "Animation/Editor/AnimationPreviewControl.h"
#include "Animation/Editor/SkeletonAsset.h"
#include "Core/Misc/SafeDestroy.h"
#include "Database/Instance.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Editor/PropertiesView.h"
#include "I18N/Text.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Resource/IResourceManager.h"
#include "Ui/Application.h"
#include "Ui/CheckBox.h"
#include "Ui/Container.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/AspectLayout.h"
#include "Ui/Splitter.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarSeparator.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/NodeMovedEvent.h"
#include "Ui/Graph/Edge.h"
#include "Ui/Graph/EdgeConnectEvent.h"
#include "Ui/Graph/EdgeDisconnectEvent.h"
#include "Ui/Graph/Pin.h"
#include "Ui/Graph/DefaultNodeShape.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimationGraphEditorPage", AnimationGraphEditorPage, editor::IEditorPage)

AnimationGraphEditorPage::AnimationGraphEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
{
}

bool AnimationGraphEditorPage::create(ui::Container* parent)
{
	m_animationGraph = m_document->getObject< AnimationGraph >(0);
	if (!m_animationGraph)
		return false;

	m_statePreviewController = new AnimationGraphPoseController(resource::Proxy< AnimationGraph >(m_animationGraph), nullptr);

	// Create state graph container.
	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut));

	// Create our custom toolbar.
	m_toolBarGraph = new ui::ToolBar();
	m_toolBarGraph->create(container);
	for (int32_t i = 0; i < 6; ++i)
		m_toolBarGraph->addImage(new ui::StyleBitmap(L"Animation.Alignment", i));
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"STATEGRAPH_ALIGN_LEFT"), 0, ui::Command(L"AnimationGraph.Editor.AlignLeft")));
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"STATEGRAPH_ALIGN_RIGHT"), 1, ui::Command(L"AnimationGraph.Editor.AlignRight")));
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"STATEGRAPH_ALIGN_TOP"), 2, ui::Command(L"AnimationGraph.Editor.AlignTop")));
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"STATEGRAPH_ALIGN_BOTTOM"), 3, ui::Command(L"AnimationGraph.Editor.AlignBottom")));
	m_toolBarGraph->addItem(new ui::ToolBarSeparator());
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"STATEGRAPH_EVEN_VERTICALLY"), 4, ui::Command(L"AnimationGraph.Editor.EvenSpaceVertically")));
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"STATEGRAPH_EVEN_HORIZONTALLY"), 5, ui::Command(L"AnimationGraph.Editor.EventSpaceHorizontally")));
	m_toolBarGraph->addEventHandler< ui::ToolBarButtonClickEvent >(this, &AnimationGraphEditorPage::eventToolBarGraphClick);

	// Create state graph editor control.
	m_editorGraph = new ui::GraphControl();
	m_editorGraph->create(container, ui::GraphControl::WsEdgeSelectable | ui::WsDoubleBuffer | ui::WsAccelerated);
	m_editorGraph->setText(L"ANIMATION STATE");
	m_editorGraph->addEventHandler< ui::MouseButtonDownEvent >(this, &AnimationGraphEditorPage::eventButtonDown);
	m_editorGraph->addEventHandler< ui::SelectionChangeEvent >(this, &AnimationGraphEditorPage::eventSelect);
	m_editorGraph->addEventHandler< ui::NodeMovedEvent >(this, &AnimationGraphEditorPage::eventNodeMoved);
	m_editorGraph->addEventHandler< ui::EdgeConnectEvent >(this, &AnimationGraphEditorPage::eventEdgeConnect);
	m_editorGraph->addEventHandler< ui::EdgeDisconnectEvent >(this, &AnimationGraphEditorPage::eventEdgeDisconnect);

	// Build popup menu.
	m_menuPopup = new ui::Menu();
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"AnimationGraph.Editor.Create"), i18n::Text(L"STATEGRAPH_CREATE_STATE")));
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"STATEGRAPH_DELETE_STATE")));
	m_menuPopup->add(new ui::MenuItem(L"-"));
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"AnimationGraph.Editor.SetRoot"), i18n::Text(L"STATEGRAPH_SET_ROOT")));

	// Create properties view.
	m_propertiesView = m_site->createPropertiesView(parent);
	m_propertiesView->addEventHandler< ui::ContentChangeEvent >(this, &AnimationGraphEditorPage::eventPropertiesChanged);
	m_site->createAdditionalPanel(m_propertiesView, 400_ut, false);

	// Create preview panel.
	m_containerPreview = new ui::Container();
	m_containerPreview->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%,*", 0_ut, 0_ut));
	m_containerPreview->setText(L"Animation Preview");

	m_toolBarPreview = new ui::ToolBar();
	m_toolBarPreview->create(m_containerPreview);
	m_toolBarPreview->addItem(new ui::ToolBarButton(L"Mesh...", ui::Command(L"AnimationGraph.Editor.BrowseMesh")));
	m_toolBarPreview->addItem(new ui::ToolBarButton(L"Skeleton...", ui::Command(L"AnimationGraph.Editor.BrowseSkeleton")));
	m_toolBarPreview->addEventHandler< ui::ToolBarButtonClickEvent >(this, &AnimationGraphEditorPage::eventToolBarPreviewClick);

	m_previewControl = new AnimationPreviewControl(m_editor);
	m_previewControl->create(m_containerPreview);
	m_previewControl->setPoseController(m_statePreviewController);

	m_previewConditions = new ui::Container();
	m_previewConditions->create(m_containerPreview, ui::WsNone, new ui::TableLayout(L"50%,50%", L"*", 0_ut, 0_ut));

	m_site->createAdditionalPanel(m_containerPreview, 450_ut, false);

	createEditorNodes(
		m_animationGraph->getStates(),
		m_animationGraph->getTransitions()
	);

	parent->update();
	m_editorGraph->center();

	updateGraph();
	updatePreviewConditions();
	bindStateNodes();

	return true;
}

void AnimationGraphEditorPage::destroy()
{
	if (m_propertiesView)
		m_site->destroyAdditionalPanel(m_propertiesView);
	if (m_containerPreview)
		m_site->destroyAdditionalPanel(m_containerPreview);

	safeDestroy(m_propertiesView);
	safeDestroy(m_containerPreview);
	safeDestroy(m_editorGraph);

	m_site = nullptr;
}

bool AnimationGraphEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	const TypeInfo* primaryType = instance->getPrimaryType();
	T_ASSERT(primaryType);

	if (is_type_of< Animation >(*primaryType))
	{
		Ref< StateNode > state = new StateNode(instance->getName(), resource::IdProxy< Animation >(instance->getGuid()));

		ui::Point absolutePosition = m_editorGraph->screenToClient(position) - m_editorGraph->getOffset();
		state->setPosition(std::pair< int, int >(absolutePosition.x, absolutePosition.y));

		m_animationGraph->addState(state);

		createEditorNode(state);
		bindStateNodes();
		updateGraph();
	}
	else if (is_type_of< mesh::MeshAsset >(*primaryType))
	{
		m_previewControl->setMesh(resource::Id< mesh::SkinnedMesh >(instance->getGuid()));
	}
	else if (is_type_of< SkeletonAsset >(*primaryType))
	{
		m_previewControl->setSkeleton(resource::Id< animation::Skeleton >(instance->getGuid()));
	}
	else
		return false;

	return true;
}

bool AnimationGraphEditorPage::handleCommand(const ui::Command& command)
{
	if (m_propertiesView->handleCommand(command))
		return true;
	
	if (command == L"Editor.SettingsChanged")
	{
		m_previewControl->updateSettings();
		m_previewControl->update();
	}
	//if (command == L"Editor.Cut" || command == L"Editor.Copy")
	//{
	//	RefArray< ui::Node > selectedNodes;
	//	if (m_editorGraph->getSelectedNodes(selectedNodes) > 0)
	//	{
	//		// Also copy edges which are affected by selected nodes.
	//		RefArray< ui::Edge > selectedEdges;
	//		m_editorGraph->getSelectedEdges(selectedEdges, true);

	//		Ref< ShaderGraphEditorClipboardData > data = new ShaderGraphEditorClipboardData();
	//
	//		ui::Rect bounds(0, 0, 0, 0);
	//		for (RefArray< ui::Node >::iterator i = selectedNodes.begin(); i != selectedNodes.end(); ++i)
	//		{
	//			Ref< Node > shaderNode = (*i)->getData< Node >(L"SHADERNODE");
	//			T_ASSERT(shaderNode);
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

	//		for (RefArray< ui::Edge >::iterator i = selectedEdges.begin(); i != selectedEdges.end(); ++i)
	//		{
	//			Ref< Edge > shaderEdge = (*i)->getData< Edge >(L"SHADEREDGE");
	//			T_ASSERT(shaderEdge);
	//			data->addEdge(shaderEdge);
	//		}

	//		ui::Application::getInstance()->getClipboard()->setObject(data);

	//		// Remove edges and nodes from graphs if user cuts.
	//		if (command == L"Editor.Cut")
	//		{
	//			// Save undo state.
	//			m_undoStack->push(m_shaderGraph);

	//			// Remove edges which are connected to any selected node, not only those who connects to both selected end nodes.
	//			selectedEdges.resize(0);
	//			m_editorGraph->getSelectedEdges(selectedEdges, false);

	//			for (RefArray< ui::Edge >::iterator i = selectedEdges.begin(); i != selectedEdges.end(); ++i)
	//			{
	//				m_shaderGraph->removeEdge((*i)->getData< Edge >(L"SHADEREDGE"));
	//				m_editorGraph->removeEdge(*i);
	//			}

	//			for (RefArray< ui::Node >::iterator i = selectedNodes.begin(); i != selectedNodes.end(); ++i)
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
	//		ui::Application::getInstance()->getClipboard()->getObject()
	//	);
	//	if (data)
	//	{
	//		// Save undo state.
	//		m_undoStack->push(m_shaderGraph);

	//		const ui::Rect& bounds = data->getBounds();

	//		ui::Size graphSize = m_editorGraph->getInnerRect().getSize();
	//		int centerLeft = (graphSize.cx - bounds.getWidth()) / 2 - m_editorGraph->getOffset().cx;
	//		int centerTop = (graphSize.cy - bounds.getHeight()) / 2 - m_editorGraph->getOffset().cy;

	//		for (RefArray< Node >::const_iterator i = data->getNodes().begin(); i != data->getNodes().end(); ++i)
	//		{
	//			std::pair< int, int > position = (*i)->getPosition();
	//			position.first = (position.first - bounds.left) + centerLeft;
	//			position.second = (position.second - bounds.top) + centerTop;
	//			(*i)->setPosition(position);

	//			// Add node to graph.
	//			m_shaderGraph->addNode(*i);
	//		}

	//		for (RefArray< Edge >::const_iterator i = data->getEdges().begin(); i != data->getEdges().end(); ++i)
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
	else if (command == L"Editor.Unselect")
	{
		m_editorGraph->deselectAllNodes();
		updateGraph();
	}
	else if (command == L"Editor.Delete")
	{
		const RefArray< ui::Node > nodes = m_editorGraph->getSelectedNodes();
		if (nodes.empty())
			return false;

		m_document->push();

		// First remove transitions which are connected to selected states.
		RefArray< ui::Edge > edges = m_editorGraph->getConnectedEdges(nodes, false);

		for (auto edge : edges)
		{
			Transition* transition = edge->getData< Transition >(L"TRANSITION");
			m_animationGraph->removeTransition(transition);
			m_editorGraph->removeEdge(edge);
		}

		// Then remove all states.
		for (auto node : nodes)
		{
			StateNode* state = node->getData< StateNode >(L"STATE");
			m_animationGraph->removeState(state);
			m_editorGraph->removeNode(node);
		}

		bindStateNodes();
		updateGraph();
		updatePreviewConditions();
	}
	else if (command == L"Editor.Undo")
	{
		if (m_document->undo())
		{
			Ref< AnimationGraph > stateGraph = m_document->getObject< AnimationGraph >(0);
			T_ASSERT(stateGraph);

			m_animationGraph = stateGraph;

			m_editorGraph->removeAllEdges();
			m_editorGraph->removeAllNodes();

			createEditorNodes(
				m_animationGraph->getStates(),
				m_animationGraph->getTransitions()
			);

			bindStateNodes();
			updateGraph();
			updatePreviewConditions();
		}
	}
	else if (command == L"Editor.Redo")
	{
		if (m_document->redo())
		{
			Ref< AnimationGraph > stateGraph = m_document->getObject< AnimationGraph >(0);
			T_ASSERT(stateGraph);

			m_animationGraph = stateGraph;

			m_editorGraph->removeAllEdges();
			m_editorGraph->removeAllNodes();

			createEditorNodes(
				m_animationGraph->getStates(),
				m_animationGraph->getTransitions()
			);

			bindStateNodes();
			updateGraph();
			updatePreviewConditions();
		}
	}
	else if (command == L"AnimationGraph.Editor.SetRoot")
	{
		const RefArray< ui::Node > selectedNodes = m_editorGraph->getSelectedNodes();
		if (selectedNodes.size() == 1)
		{
			Ref< StateNode > state = selectedNodes.front()->getData< StateNode >(L"STATE");
			T_ASSERT(state);

			m_animationGraph->setRootState(state);

			// Update color to show which node is root.
			for (auto node : m_editorGraph->getNodes())
				node->setShape(new ui::DefaultNodeShape(
					node == selectedNodes.front() ? ui::DefaultNodeShape::StDefault : ui::DefaultNodeShape::StExternal
				));
		}
	}
	else if (command == L"AnimationGraph.Editor.AlignLeft")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnLeft);
	}
	else if (command == L"AnimationGraph.Editor.AlignRight")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnRight);
	}
	else if (command == L"AnimationGraph.Editor.AlignTop")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnTop);
	}
	else if (command == L"AnimationGraph.Editor.AlignBottom")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnBottom);
	}
	else if (command == L"AnimationGraph.Editor.EvenSpaceVertically")
	{
		m_document->push();
		m_editorGraph->evenSpace(ui::GraphControl::EsVertically);
	}
	else if (command == L"AnimationGraph.Editor.EventSpaceHorizontally")
	{
		m_document->push();
		m_editorGraph->evenSpace(ui::GraphControl::EsHorizontally);
	}
	else if (command == L"AnimationGraph.Editor.BrowseMesh")
	{
		Ref< db::Instance > meshInstance = m_editor->browseInstance(type_of< mesh::MeshAsset >());
		if (meshInstance)
		{
			m_previewControl->setMesh(resource::Id< mesh::SkinnedMesh >(meshInstance->getGuid()));
		}
	}
	else if (command == L"AnimationGraph.Editor.BrowseSkeleton")
	{
		Ref< db::Instance > skeletonInstance = m_editor->browseInstance(type_of< animation::SkeletonAsset >());
		if (skeletonInstance)
		{
			m_previewControl->setSkeleton(resource::Id< Skeleton >(skeletonInstance->getGuid()));
		}
	}
	else
		return false;

	m_editorGraph->update();
	return true;
}

void AnimationGraphEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	m_previewControl->getResourceManager()->reload(eventId, false);
}

void AnimationGraphEditorPage::bindStateNodes()
{
	for (auto state : m_animationGraph->getStates())
		state->bind(m_previewControl->getResourceManager());
}

void AnimationGraphEditorPage::createEditorNodes(const RefArray< StateNode >& states, const RefArray< Transition >& transitions)
{
	std::map< const StateNode*, ui::Node* > nodeMap;

	// Create editor nodes for each state.
	for (auto state : states)
	{
		Ref< ui::Node > node = createEditorNode(state);
		nodeMap[state] = node;
	}

	// Create editor edges for each transition.
	for (auto transition : transitions)
	{
		StateNode* from = transition->from();
		StateNode* to = transition->to();

		ui::Node* fromNode = nodeMap[from];
		ui::Node* toNode = nodeMap[to];

		if (!fromNode || !toNode)
			continue;

		ui::Pin* fromPin = fromNode->findOutputPin(L"Leave");
		T_ASSERT(fromPin);

		ui::Pin* toPin = toNode->findInputPin(L"Enter");
		T_ASSERT(toPin);

		Ref< ui::Edge > transitionEdge = new ui::Edge(fromPin, toPin);
		transitionEdge->setData(L"TRANSITION", transition);
		m_editorGraph->addEdge(transitionEdge);
	}
}

Ref< ui::Node > AnimationGraphEditorPage::createEditorNode(StateNode* state)
{
	Ref< ui::INodeShape > shape = new ui::DefaultNodeShape(
		m_animationGraph->getRootState() == state ? ui::DefaultNodeShape::StDefault : ui::DefaultNodeShape::StExternal
	);

	Ref< ui::Node > node = m_editorGraph->createNode(
		state->getName(),
		L"",
		ui::UnitPoint(
			ui::Unit(state->getPosition().first),
			ui::Unit(state->getPosition().second)
		),
		shape
	);
	node->setData(L"STATE", state);
	node->createInputPin(L"Enter", Guid(), true, false);
	node->createOutputPin(L"Leave", Guid());

	return node;
}

void AnimationGraphEditorPage::createState(const ui::Point& at)
{
	Ref< StateNode > state = new StateNode(i18n::Text(L"STATEGRAPH_UNNAMED"), resource::IdProxy< Animation >());
	state->setPosition(std::pair< int, int >(at.x, at.y));
	m_animationGraph->addState(state);

	createEditorNode(state);
	bindStateNodes();
	updateGraph();
}

void AnimationGraphEditorPage::updateGraph()
{
	m_editorGraph->update();
}

void AnimationGraphEditorPage::updatePreviewConditions()
{
	std::map< std::wstring, bool > conditions;

	// Collect all condition variables.
	for (auto transition : m_animationGraph->getTransitions())
	{
		std::wstring c = transition->getCondition();
		if (!c.empty())
		{
			if (c[0] == L'!')
				c = c.substr(1);

			conditions[c] = false;
		}
	}

	// Keep all existing condition states.
	for (ui::Widget* it = m_previewConditions->getFirstChild(); it; it = it->getNextSibling())
	{
		ui::CheckBox* condition = mandatory_non_null_type_cast< ui::CheckBox* >(it);
		if (conditions.find(condition->getText()) != conditions.end())
			conditions[condition->getText()] = condition->isChecked();
	}

	// Destroy all checkboxes.
	while (m_previewConditions->getFirstChild())
		m_previewConditions->getFirstChild()->destroy();

	// Recreate checkboxes.
	for (auto i = conditions.begin(); i != conditions.end(); ++i)
	{
		Ref< ui::CheckBox > cb = new ui::CheckBox();
		cb->create(m_previewConditions, i->first, i->second);
		cb->addEventHandler< ui::ButtonClickEvent >(this, &AnimationGraphEditorPage::eventPreviewConditionClick);
	}

	m_containerPreview->update();
}

void AnimationGraphEditorPage::eventToolBarGraphClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	handleCommand(command);
}

void AnimationGraphEditorPage::eventToolBarPreviewClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	handleCommand(command);
}

void AnimationGraphEditorPage::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtRight)
		return;

	const ui::MenuItem* selected = m_menuPopup->showModal(m_editorGraph, event->getPosition());
	if (!selected)
		return;

	const ui::Command& command = selected->getCommand();

	if (command == L"AnimationGraph.Editor.Create")
		createState(event->getPosition() - m_editorGraph->getOffset());
	else
		handleCommand(command);
}

void AnimationGraphEditorPage::eventSelect(ui::SelectionChangeEvent* event)
{
	const RefArray< ui::Node > nodes = m_editorGraph->getSelectedNodes();
	const RefArray< ui::Edge > edges = m_editorGraph->getSelectedEdges();

	if (nodes.size() == 1)
	{
		StateNode* state = nodes[0]->getData< StateNode >(L"STATE");
		T_ASSERT(state);

		Ref< AnimationGraph > stateGraph = new AnimationGraph();
		stateGraph->addState(state);
		stateGraph->addTransition(new Transition(state, state));
		stateGraph->setRootState(state);

		m_propertiesView->setPropertyObject(state);
		m_previewControl->setPoseController(new AnimationGraphPoseController(resource::Proxy< AnimationGraph >(stateGraph), nullptr));
	}
	else if (edges.size() == 1)
	{
		Transition* transition = edges[0]->getData< Transition >(L"TRANSITION");
		T_ASSERT(transition);

		m_propertiesView->setPropertyObject(transition);
		m_previewControl->setPoseController(m_statePreviewController);
	}
	else
	{
		m_propertiesView->setPropertyObject(nullptr);
		m_previewControl->setPoseController(m_statePreviewController);
	}
}

void AnimationGraphEditorPage::eventNodeMoved(ui::NodeMovedEvent* event)
{
	ui::Node* node = event->getNode();
	T_ASSERT(node);

	// Get state from editor node.
	StateNode* state = node->getData< StateNode >(L"STATE");
	T_ASSERT(state);

	ui::UnitPoint position = node->getPosition();
	if (position.x.get() != state->getPosition().first || position.y.get() != state->getPosition().second)
	{
		state->setPosition(std::pair< int, int >(
			node->getPosition().x.get(),
			node->getPosition().y.get()
		));
	}

	// Update properties.
	if (node->isSelected())
		m_propertiesView->setPropertyObject(state);
}

void AnimationGraphEditorPage::eventEdgeConnect(ui::EdgeConnectEvent* event)
{
	ui::Edge* edge = event->getEdge();

	ui::Pin* leavePin = edge->getSourcePin();
	T_ASSERT(leavePin);

	ui::Pin* enterPin = edge->getDestinationPin();
	T_ASSERT(enterPin);

	StateNode* leaveState = leavePin->getNode()->getData< StateNode >(L"STATE");
	T_ASSERT(leaveState);

	StateNode* enterState = enterPin->getNode()->getData< StateNode >(L"STATE");
	T_ASSERT(enterState);

	Ref< Transition > transition = new Transition(leaveState, enterState);
	m_animationGraph->addTransition(transition);

	edge->setData(L"TRANSITION", transition);
	m_editorGraph->addEdge(edge);

	updateGraph();
}

void AnimationGraphEditorPage::eventEdgeDisconnect(ui::EdgeDisconnectEvent* event)
{
	Ref< ui::Edge > edge = event->getEdge();

	Transition* transition = checked_type_cast< Transition* >(edge->getData(L"TRANSITION"));
	m_animationGraph->removeTransition(transition);

	updateGraph();
}

void AnimationGraphEditorPage::eventPropertiesChanged(ui::ContentChangeEvent* event)
{
	// Refresh editor nodes.
	for (auto node : m_editorGraph->getNodes())
	{
		StateNode* state = node->getData< StateNode >(L"STATE");
		node->setTitle(state->getName());

		const auto& position = state->getPosition();
		node->setPosition(ui::UnitPoint(
			ui::Unit(position.first),
			ui::Unit(position.second)
		));
	}

	updateGraph();
	updatePreviewConditions();
}

void AnimationGraphEditorPage::eventPreviewConditionClick(ui::ButtonClickEvent* event)
{
	ui::CheckBox* cb = mandatory_non_null_type_cast< ui::CheckBox* >(event->getSender());
	m_statePreviewController->setCondition(cb->getText(), cb->isChecked(), false);
}

}
