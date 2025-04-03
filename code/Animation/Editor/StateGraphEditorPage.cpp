/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/StateGraphEditorPage.h"

#include "Animation/Animation/Animation.h"
#include "Animation/Animation/AnimationGraphPoseController.h"
#include "Animation/Animation/AnimationGraphPoseControllerData.h"
#include "Animation/Animation/RtStateGraph.h"
#include "Animation/Animation/RtStateGraphData.h"
#include "Animation/Editor/AnimationAsset.h"
#include "Animation/Editor/AnimationPreviewControl.h"
#include "Animation/Editor/SkeletonAsset.h"
#include "Animation/Editor/StateGraph.h"
#include "Animation/Editor/StateGraphCompiler.h"
#include "Animation/Editor/StateNodeAnimation.h"
#include "Animation/Editor/StateNodeAny.h"
#include "Animation/Editor/StateNodeController.h"
#include "Animation/Editor/StateTransition.h"
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
#include "Ui/AspectLayout.h"
#include "Ui/CheckBox.h"
#include "Ui/Container.h"
#include "Ui/Graph/DefaultNodeShape.h"
#include "Ui/Graph/Edge.h"
#include "Ui/Graph/EdgeConnectEvent.h"
#include "Ui/Graph/EdgeDisconnectEvent.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/NodeMovedEvent.h"
#include "Ui/Graph/Pin.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/Splitter.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarSeparator.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.StateGraphEditorPage", StateGraphEditorPage, editor::IEditorPage)

StateGraphEditorPage::StateGraphEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
	: m_editor(editor)
	, m_site(site)
	, m_document(document)
{
}

bool StateGraphEditorPage::create(ui::Container* parent)
{
	m_stateGraph = m_document->getObject< StateGraph >(0);
	if (!m_stateGraph)
		return false;

	// Create state graph container.
	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut));

	// Create our custom toolbar.
	m_toolBarGraph = new ui::ToolBar();
	m_toolBarGraph->create(container);
	for (int32_t i = 0; i < 6; ++i)
		m_toolBarGraph->addImage(new ui::StyleBitmap(L"Animation.Alignment", i));
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"STATEGRAPH_ALIGN_LEFT"), 0, ui::Command(L"StateGraph.Editor.AlignLeft")));
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"STATEGRAPH_ALIGN_RIGHT"), 1, ui::Command(L"StateGraph.Editor.AlignRight")));
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"STATEGRAPH_ALIGN_TOP"), 2, ui::Command(L"StateGraph.Editor.AlignTop")));
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"STATEGRAPH_ALIGN_BOTTOM"), 3, ui::Command(L"StateGraph.Editor.AlignBottom")));
	m_toolBarGraph->addItem(new ui::ToolBarSeparator());
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"STATEGRAPH_EVEN_VERTICALLY"), 4, ui::Command(L"StateGraph.Editor.EvenSpaceVertically")));
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"STATEGRAPH_EVEN_HORIZONTALLY"), 5, ui::Command(L"StateGraph.Editor.EventSpaceHorizontally")));
	m_toolBarGraph->addEventHandler< ui::ToolBarButtonClickEvent >(this, &StateGraphEditorPage::eventToolBarGraphClick);

	// Create state graph editor control.
	m_editorGraph = new ui::GraphControl();
	m_editorGraph->create(container, ui::GraphControl::WsEdgeSelectable | ui::WsDoubleBuffer | ui::WsAccelerated);
	m_editorGraph->setText(L"ANIMATION STATE");
	m_editorGraph->addEventHandler< ui::MouseButtonDownEvent >(this, &StateGraphEditorPage::eventButtonDown);
	m_editorGraph->addEventHandler< ui::SelectionChangeEvent >(this, &StateGraphEditorPage::eventSelect);
	m_editorGraph->addEventHandler< ui::NodeMovedEvent >(this, &StateGraphEditorPage::eventNodeMoved);
	m_editorGraph->addEventHandler< ui::EdgeConnectEvent >(this, &StateGraphEditorPage::eventEdgeConnect);
	m_editorGraph->addEventHandler< ui::EdgeDisconnectEvent >(this, &StateGraphEditorPage::eventEdgeDisconnect);

	// Build pop up menu.
	m_menuPopup = new ui::Menu();
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"StateGraph.Editor.CreateStateAnimation"), i18n::Text(L"STATEGRAPH_CREATE_STATE_ANIMATION")));
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"StateGraph.Editor.CreateStateAny"), i18n::Text(L"STATEGRAPH_CREATE_STATE_ANY")));
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"StateGraph.Editor.CreateStateController"), i18n::Text(L"STATEGRAPH_CREATE_STATE_CONTROLLER")));
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"STATEGRAPH_DELETE_STATE")));
	m_menuPopup->add(new ui::MenuItem(L"-"));
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"StateGraph.Editor.SetRoot"), i18n::Text(L"STATEGRAPH_SET_ROOT")));

	// Create properties view.
	m_propertiesView = m_site->createPropertiesView(parent);
	m_propertiesView->addEventHandler< ui::ContentChangeEvent >(this, &StateGraphEditorPage::eventPropertiesChanged);
	m_site->createAdditionalPanel(m_propertiesView, 400_ut, false);

	// Create preview panel.
	m_containerPreview = new ui::Container();
	m_containerPreview->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%,*", 0_ut, 0_ut));
	m_containerPreview->setText(L"Animation Preview");

	m_toolBarPreview = new ui::ToolBar();
	m_toolBarPreview->create(m_containerPreview);
	m_toolBarPreview->addItem(new ui::ToolBarButton(L"Mesh...", ui::Command(L"StateGraph.Editor.BrowseMesh")));
	m_toolBarPreview->addItem(new ui::ToolBarButton(L"Skeleton...", ui::Command(L"StateGraph.Editor.BrowseSkeleton")));
	m_toolBarPreview->addItem(new ui::ToolBarButton(L"Capture preview transform", ui::Command(L"StateGraph.Editor.CapturePreviewTransform")));
	m_toolBarPreview->addEventHandler< ui::ToolBarButtonClickEvent >(this, &StateGraphEditorPage::eventToolBarPreviewClick);

	m_previewControl = new AnimationPreviewControl(m_editor);
	m_previewControl->create(m_containerPreview);
	m_previewControl->setView({ .position = m_stateGraph->getPreviewPosition(),
		.head = m_stateGraph->getPreviewAngles().x(),
		.pitch = m_stateGraph->getPreviewAngles().y() });

	m_previewConditions = new ui::Container();
	m_previewConditions->create(m_containerPreview, ui::WsNone, new ui::TableLayout(L"50%,50%", L"*", 0_ut, 0_ut));

	m_site->createAdditionalPanel(m_containerPreview, 450_ut, false);

	createEditorNodes(
		m_stateGraph->getStates(),
		m_stateGraph->getTransitions());

	parent->update();
	m_editorGraph->center();

	updateGraph();
	updatePreview(nullptr);
	updatePreviewConditions();

	m_propertiesView->setPropertyObject(m_stateGraph);

	return true;
}

void StateGraphEditorPage::destroy()
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

bool StateGraphEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	const ui::Point absolutePosition = m_editorGraph->screenToClient(position) - m_editorGraph->getOffset();

	const TypeInfo* primaryType = instance->getPrimaryType();
	T_ASSERT(primaryType);

	if (is_type_of< AnimationAsset >(*primaryType))
	{
		Ref< StateNodeAnimation > state = new StateNodeAnimation(instance->getName(), resource::Id< Animation >(instance->getGuid()));
		state->setPosition(std::pair< int, int >(absolutePosition.x, absolutePosition.y));
		m_stateGraph->addState(state);

		createEditorNode(state);
		updateGraph();
	}
	else if (is_type_of< StateGraph >(*primaryType))
	{
		Ref< StateNodeController > state = new StateNodeController(
			instance->getName(),
			new AnimationGraphPoseControllerData(resource::Id< RtStateGraph >(instance->getGuid())));
		state->setPosition(std::pair< int, int >(absolutePosition.x, absolutePosition.y));
		m_stateGraph->addState(state);

		createEditorNode(state);
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

bool StateGraphEditorPage::handleCommand(const ui::Command& command)
{
	if (m_propertiesView->handleCommand(command))
		return true;

	if (command == L"Editor.SettingsChanged")
	{
		m_previewControl->updateSettings();
		m_previewControl->update();
	}
	// if (command == L"Editor.Cut" || command == L"Editor.Copy")
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
	// else if (command == L"Editor.Paste")
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
			StateTransition* transition = edge->getData< StateTransition >(L"TRANSITION");
			m_stateGraph->removeTransition(transition);
			m_editorGraph->removeEdge(edge);
		}

		// Then remove all states.
		for (auto node : nodes)
		{
			StateNode* state = node->getData< StateNode >(L"STATE");
			m_stateGraph->removeState(state);
			m_editorGraph->removeNode(node);
		}

		updateGraph();
		updatePreviewConditions();
	}
	else if (command == L"Editor.Undo")
	{
		if (m_document->undo())
		{
			Ref< StateGraph > stateGraph = m_document->getObject< StateGraph >(0);
			T_ASSERT(stateGraph);

			m_stateGraph = stateGraph;

			m_editorGraph->removeAllEdges();
			m_editorGraph->removeAllNodes();

			createEditorNodes(
				m_stateGraph->getStates(),
				m_stateGraph->getTransitions());

			updateGraph();
			updatePreviewConditions();
		}
	}
	else if (command == L"Editor.Redo")
	{
		if (m_document->redo())
		{
			Ref< StateGraph > stateGraph = m_document->getObject< StateGraph >(0);
			T_ASSERT(stateGraph);

			m_stateGraph = stateGraph;

			m_editorGraph->removeAllEdges();
			m_editorGraph->removeAllNodes();

			createEditorNodes(
				m_stateGraph->getStates(),
				m_stateGraph->getTransitions());

			updateGraph();
			updatePreviewConditions();
		}
	}
	else if (command == L"StateGraph.Editor.SetRoot")
	{
		const RefArray< ui::Node > selectedNodes = m_editorGraph->getSelectedNodes();
		if (selectedNodes.size() == 1)
		{
			Ref< StateNode > state = selectedNodes.front()->getData< StateNode >(L"STATE");
			T_ASSERT(state);

			m_stateGraph->setRootState(state);

			// Update color to show which node is root.
			for (auto node : m_editorGraph->getNodes())
				node->setShape(new ui::DefaultNodeShape(
					node == selectedNodes.front() ? ui::DefaultNodeShape::StDefault : ui::DefaultNodeShape::StExternal));
		}
	}
	else if (command == L"StateGraph.Editor.AlignLeft")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnLeft);
	}
	else if (command == L"StateGraph.Editor.AlignRight")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnRight);
	}
	else if (command == L"StateGraph.Editor.AlignTop")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnTop);
	}
	else if (command == L"StateGraph.Editor.AlignBottom")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnBottom);
	}
	else if (command == L"StateGraph.Editor.EvenSpaceVertically")
	{
		m_document->push();
		m_editorGraph->evenSpace(ui::GraphControl::EsVertically);
	}
	else if (command == L"StateGraph.Editor.EventSpaceHorizontally")
	{
		m_document->push();
		m_editorGraph->evenSpace(ui::GraphControl::EsHorizontally);
	}
	else if (command == L"StateGraph.Editor.BrowseMesh")
	{
		Ref< db::Instance > meshInstance = m_editor->browseInstance(type_of< mesh::MeshAsset >());
		if (meshInstance)
			m_previewControl->setMesh(resource::Id< mesh::SkinnedMesh >(meshInstance->getGuid()));
	}
	else if (command == L"StateGraph.Editor.BrowseSkeleton")
	{
		Ref< db::Instance > skeletonInstance = m_editor->browseInstance(type_of< animation::SkeletonAsset >());
		if (skeletonInstance)
			m_previewControl->setSkeleton(resource::Id< Skeleton >(skeletonInstance->getGuid()));
	}
	else if (command == L"StateGraph.Editor.CapturePreviewTransform")
	{
		const AnimationPreviewControl::View view = m_previewControl->getView();
		m_stateGraph->setPreviewPosition(view.position);
		m_stateGraph->setPreviewAngles(Vector4(view.head, view.pitch, 0.0f));
	}
	else
		return false;

	m_editorGraph->update();
	return true;
}

void StateGraphEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	m_previewControl->getResourceManager()->reload(eventId, false);
}

void StateGraphEditorPage::createEditorNodes(const RefArray< StateNode >& states, const RefArray< StateTransition >& transitions)
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
		transitionEdge->setText(transition->getCondition());
		m_editorGraph->addEdge(transitionEdge);
	}
}

Ref< ui::Node > StateGraphEditorPage::createEditorNode(StateNode* state)
{
	Ref< ui::INodeShape > shape = new ui::DefaultNodeShape(
		m_stateGraph->getRootState() == state ? ui::DefaultNodeShape::StDefault : ui::DefaultNodeShape::StExternal);

	Ref< ui::Node > node = m_editorGraph->createNode(
		!state->getName().empty() ? state->getName() : std::wstring(i18n::Text(L"STATEGRAPH_UNNAMED")),
		L"",
		ui::UnitPoint(
			ui::Unit(state->getPosition().first),
			ui::Unit(state->getPosition().second)),
		shape);
	node->setData(L"STATE", state);

	// Do not create input pin on "any" node since it doesn't make sense.
	if (!is_a< StateNodeAny >(state))
		node->createInputPin(L"Enter", Guid(), true, false);

	node->createOutputPin(L"Leave", Guid());

	return node;
}

void StateGraphEditorPage::createState(const ui::Point& at, const TypeInfo& stateType)
{
	Ref< StateNode > state = dynamic_type_cast< StateNode* >(stateType.createInstance());
	state->setPosition(std::pair< int, int >(at.x, at.y));
	m_stateGraph->addState(state);

	createEditorNode(state);
	updateGraph();
}

void StateGraphEditorPage::updateGraph()
{
	m_editorGraph->update();
}

void StateGraphEditorPage::updatePreview(const StateGraph* stateGraph)
{
	m_previewControl->setSkeleton(m_stateGraph->getPreviewSkeleton());
	m_previewControl->setMesh(m_stateGraph->getPreviewMesh());
	m_previewControl->setPoseController(nullptr);

	Ref< const RtStateGraphData > rtsgd = StateGraphCompiler().compile(stateGraph ? stateGraph : m_stateGraph);
	if (rtsgd)
	{
		Ref< RtStateGraph > rtsg = rtsgd->createInstance(m_previewControl->getResourceManager());
		if (rtsg)
			m_previewControl->setPoseController(new AnimationGraphPoseController(resource::Proxy< RtStateGraph >(rtsg), nullptr));
	}
}

void StateGraphEditorPage::updatePreviewConditions()
{
	std::map< std::wstring, bool > conditions;

	// Collect all condition variables.
	for (auto transition : m_stateGraph->getTransitions())
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
		cb->addEventHandler< ui::ButtonClickEvent >(this, &StateGraphEditorPage::eventPreviewConditionClick);
	}

	m_containerPreview->update();
}

void StateGraphEditorPage::eventToolBarGraphClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	handleCommand(command);
}

void StateGraphEditorPage::eventToolBarPreviewClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	handleCommand(command);
}

void StateGraphEditorPage::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtRight)
		return;

	const ui::MenuItem* selected = m_menuPopup->showModal(m_editorGraph, event->getPosition());
	if (!selected)
		return;

	const ui::Command& command = selected->getCommand();

	if (command == L"StateGraph.Editor.CreateStateAnimation")
		createState(event->getPosition() - m_editorGraph->getOffset(), type_of< StateNodeAnimation >());
	else if (command == L"StateGraph.Editor.CreateStateAny")
		createState(event->getPosition() - m_editorGraph->getOffset(), type_of< StateNodeAny >());
	else if (command == L"StateGraph.Editor.CreateStateController")
		createState(event->getPosition() - m_editorGraph->getOffset(), type_of< StateNodeController >());
	else
		handleCommand(command);
}

void StateGraphEditorPage::eventSelect(ui::SelectionChangeEvent* event)
{
	const RefArray< ui::Node > nodes = m_editorGraph->getSelectedNodes();
	const RefArray< ui::Edge > edges = m_editorGraph->getSelectedEdges();

	if (nodes.size() == 1)
	{
		StateNode* state = nodes[0]->getData< StateNode >(L"STATE");
		T_FATAL_ASSERT(state);

		Ref< StateGraph > stateGraph = new StateGraph();
		stateGraph->addState(state);
		stateGraph->addTransition(new StateTransition(state, state));
		stateGraph->setRootState(state);
		updatePreview(stateGraph);
		m_propertiesView->setPropertyObject(state);
	}
	else if (edges.size() == 1)
	{
		StateTransition* transition = edges[0]->getData< StateTransition >(L"TRANSITION");
		T_FATAL_ASSERT(transition);

		updatePreview(nullptr);
		m_propertiesView->setPropertyObject(transition);
	}
	else
	{
		updatePreview(nullptr);
		m_propertiesView->setPropertyObject(m_stateGraph);
	}
}

void StateGraphEditorPage::eventNodeMoved(ui::NodeMovedEvent* event)
{
	ui::Node* node = event->getNode();
	T_ASSERT(node);

	// Get state from editor node.
	StateNode* state = node->getData< StateNode >(L"STATE");
	T_ASSERT(state);

	ui::UnitPoint position = node->getPosition();
	if (position.x.get() != state->getPosition().first || position.y.get() != state->getPosition().second)
		state->setPosition(std::pair< int, int >(
			node->getPosition().x.get(),
			node->getPosition().y.get()));

	// Update properties.
	if (node->isSelected())
		m_propertiesView->setPropertyObject(state);
}

void StateGraphEditorPage::eventEdgeConnect(ui::EdgeConnectEvent* event)
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

	Ref< StateTransition > transition = new StateTransition(leaveState, enterState);
	m_stateGraph->addTransition(transition);

	edge->setData(L"TRANSITION", transition);
	m_editorGraph->addEdge(edge);

	updateGraph();
	updatePreview(nullptr);
}

void StateGraphEditorPage::eventEdgeDisconnect(ui::EdgeDisconnectEvent* event)
{
	Ref< ui::Edge > edge = event->getEdge();

	StateTransition* transition = mandatory_non_null_type_cast< StateTransition* >(edge->getData(L"TRANSITION"));
	m_stateGraph->removeTransition(transition);

	updateGraph();
	updatePreview(nullptr);
}

void StateGraphEditorPage::eventPropertiesChanged(ui::ContentChangeEvent* event)
{
	// Refresh editor nodes.
	for (auto node : m_editorGraph->getNodes())
	{
		StateNode* state = node->getData< StateNode >(L"STATE");
		node->setTitle(state->getName());

		const auto& position = state->getPosition();
		node->setPosition(ui::UnitPoint(
			ui::Unit(position.first),
			ui::Unit(position.second)));
	}

	updateGraph();
	updatePreview(nullptr);
	updatePreviewConditions();
}

void StateGraphEditorPage::eventPreviewConditionClick(ui::ButtonClickEvent* event)
{
	const ui::CheckBox* cb = mandatory_non_null_type_cast< ui::CheckBox* >(event->getSender());
	m_previewControl->setParameterValue(cb->getText(), cb->isChecked());
}
}
