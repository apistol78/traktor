/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/DeepClone.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Editor/PropertiesView.h"
#include "I18N/Text.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/Image2/IImgStep.h"
#include "Render/Editor/Image2/ImageGraphAsset.h"
#include "Render/Editor/Image2/ImageGraphClipboardData.h"
#include "Render/Editor/Image2/ImageGraphEditorPage.h"
#include "Render/Editor/Image2/ImgInput.h"
#include "Render/Editor/Image2/ImgOutput.h"
#include "Render/Editor/Image2/ImgPass.h"
#include "Render/Editor/Image2/ImgStructBuffer.h"
#include "Render/Editor/Image2/ImgTargetSet.h"
#include "Render/Editor/Image2/ImgTexture.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Container.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Graph/DefaultNodeShape.h"
#include "Ui/Graph/Edge.h"
#include "Ui/Graph/EdgeConnectEvent.h"
#include "Ui/Graph/EdgeDisconnectEvent.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/InputNodeShape.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/NodeMovedEvent.h"
#include "Ui/Graph/OutputNodeShape.h"
#include "Ui/Graph/Pin.h"
#include "Ui/Graph/SelectEvent.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarSeparator.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageGraphEditorPage", ImageGraphEditorPage, editor::IEditorPage)

ImageGraphEditorPage::ImageGraphEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
{
}

bool ImageGraphEditorPage::create(ui::Container* parent)
{
	m_imageGraph = m_document->getObject< ImageGraphAsset >(0);
	if (!m_imageGraph)
		return false;

	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut));

	// Create our custom toolbar.
	m_toolBar = new ui::ToolBar();
	m_toolBar->create(container);
	for (int32_t i = 0; i < 18; ++i)
		m_toolBar->addImage(new ui::StyleBitmap(L"Shader.Tools", i));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"IMAGEGRAPH_CENTER"), 7, ui::Command(L"ImageGraph.Editor.Center")));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"IMAGEGRAPH_ALIGN_LEFT"), 0, ui::Command(L"ImageGraph.Editor.AlignLeft")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"IMAGEGRAPH_ALIGN_RIGHT"), 1, ui::Command(L"ImageGraph.Editor.AlignRight")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"IMAGEGRAPH_ALIGN_TOP"), 2, ui::Command(L"ImageGraph.Editor.AlignTop")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"IMAGEGRAPH_ALIGN_BOTTOM"), 3, ui::Command(L"ImageGraph.Editor.AlignBottom")));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"IMAGEGRAPH_EVEN_VERTICALLY"), 4, ui::Command(L"ImageGraph.Editor.EvenSpaceVertically")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"IMAGEGRAPH_EVEN_HORIZONTALLY"), 5, ui::Command(L"ImageGraph.Editor.EventSpaceHorizontally")));
	m_toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &ImageGraphEditorPage::eventToolClick);

	m_editorGraph = new ui::GraphControl();
	m_editorGraph->create(container);
	m_editorGraph->setText(L"IMAGE");
	m_editorGraph->addEventHandler< ui::MouseButtonDownEvent >(this, &ImageGraphEditorPage::eventButtonDown);
	m_editorGraph->addEventHandler< ui::SelectEvent >(this, &ImageGraphEditorPage::eventSelect);
	m_editorGraph->addEventHandler< ui::NodeMovedEvent >(this, &ImageGraphEditorPage::eventNodeMoved);
	m_editorGraph->addEventHandler< ui::EdgeConnectEvent >(this, &ImageGraphEditorPage::eventEdgeConnect);
	m_editorGraph->addEventHandler< ui::EdgeDisconnectEvent >(this, &ImageGraphEditorPage::eventEdgeDisconnect);

	// Create properties view.
	m_propertiesView = m_site->createPropertiesView(parent);
	m_propertiesView->addEventHandler< ui::ContentChangingEvent >(this, &ImageGraphEditorPage::eventPropertiesChanging);
	m_propertiesView->addEventHandler< ui::ContentChangeEvent >(this, &ImageGraphEditorPage::eventPropertiesChanged);
	m_site->createAdditionalPanel(m_propertiesView, 400_ut, false);

	m_menuPopup = new ui::Menu();
	Ref< ui::MenuItem > menuItemCreate = new ui::MenuItem(i18n::Text(L"IMAGEGRAPH_CREATE"));
	menuItemCreate->add(new ui::MenuItem(ui::Command(L"ImageGraph.Editor.AddInput"), i18n::Text(L"IMAGEGRAPH_CREATE_INPUT")));
	menuItemCreate->add(new ui::MenuItem(ui::Command(L"ImageGraph.Editor.AddOutput"), i18n::Text(L"IMAGEGRAPH_CREATE_OUTPUT")));
	menuItemCreate->add(new ui::MenuItem(ui::Command(L"ImageGraph.Editor.AddPass"), i18n::Text(L"IMAGEGRAPH_CREATE_PASS")));
	menuItemCreate->add(new ui::MenuItem(ui::Command(L"ImageGraph.Editor.AddTargetSet"), i18n::Text(L"IMAGEGRAPH_CREATE_TARGETSET")));
	menuItemCreate->add(new ui::MenuItem(ui::Command(L"ImageGraph.Editor.AddTexture"), i18n::Text(L"IMAGEGRAPH_CREATE_TEXTURE")));
	menuItemCreate->add(new ui::MenuItem(ui::Command(L"ImageGraph.Editor.AddStructBuffer"), i18n::Text(L"IMAGEGRAPH_CREATE_STRUCT_BUFFER")));
	m_menuPopup->add(menuItemCreate);
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"IMAGEGRAPH_DELETE")));

	createEditorGraph();
	return true;
}

void ImageGraphEditorPage::destroy()
{
	if (m_propertiesView)
		m_site->destroyAdditionalPanel(m_propertiesView);

	safeDestroy(m_propertiesView);	
	safeDestroy(m_editorGraph);

	m_site = nullptr;
}

bool ImageGraphEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool ImageGraphEditorPage::handleCommand(const ui::Command& command)
{
	if (m_propertiesView->handleCommand(command))
		return true;
	
	if (command == L"Editor.Cut" || command == L"Editor.Copy")
	{
		 const RefArray< ui::Node > selectedNodes = m_editorGraph->getSelectedNodes();
		 if (!selectedNodes.empty())
		 {
		 	// Also copy edges which are affected by selected nodes.
		 	RefArray< ui::Edge > selectedEdges = m_editorGraph->getConnectedEdges(selectedNodes, true);

		 	Ref< ImageGraphClipboardData > data = new ImageGraphClipboardData();

		 	ui::UnitRect bounds(0_ut, 0_ut, 0_ut, 0_ut);
		 	for (auto i = selectedNodes.begin(); i != selectedNodes.end(); ++i)
		 	{
		 		Ref< Node > shaderNode = (*i)->getData< Node >(L"IMGNODE");
		 		T_ASSERT(shaderNode);
		 		data->addNode(shaderNode);

		 		if (i != selectedNodes.begin())
		 		{
		 			const ui::UnitRect rc = (*i)->calculateRect();
		 			bounds.left = std::min(bounds.left, rc.left);
		 			bounds.top = std::min(bounds.top, rc.top);
		 			bounds.right = std::max(bounds.right, rc.right);
		 			bounds.bottom = std::max(bounds.bottom, rc.bottom);
		 		}
		 		else
		 			bounds = (*i)->calculateRect();
		 	}

		 	data->setBounds(bounds);

		 	for (auto selectedEdge : selectedEdges)
		 	{
		 		Ref< Edge > edge = selectedEdge->getData< Edge >(L"IMGEDGE");
		 		T_ASSERT(edge);
		 		data->addEdge(edge);
		 	}

		 	ui::Application::getInstance()->getClipboard()->setObject(data);

		 	// Remove edges and nodes from graphs if user cuts.
		 	if (command == L"Editor.Cut")
		 	{
		 		// Save undo state.
		 		m_document->push();

		 		// Remove edges which are connected to any selected node, not only those who connects to both selected end nodes.
				selectedEdges = m_editorGraph->getConnectedEdges(selectedNodes, false);

		 		for (auto selectedEdge : selectedEdges)
		 		{
		 			m_imageGraph->removeEdge(selectedEdge->getData< Edge >(L"IMGEDGE"));
		 			m_editorGraph->removeEdge(selectedEdge);
		 		}

		 		for (auto selectedNode : selectedNodes)
		 		{
		 			m_imageGraph->removeNode(selectedNode->getData< Node >(L"IMGNODE"));
		 			m_editorGraph->removeNode(selectedNode);
		 		}
		 	}
		 }
	}
	else if (command == L"Editor.Paste")
	{
		 Ref< ImageGraphClipboardData > data = dynamic_type_cast< ImageGraphClipboardData* >(
		 	ui::Application::getInstance()->getClipboard()->getObject()
		 );
		 if (data)
		 {
		 	// Save undo state.
		 	m_document->push();

		 	const ui::UnitRect& bounds = data->getBounds();

		 	ui::Rect rcClient = m_editorGraph->getInnerRect();
		 	ui::Point center = m_editorGraph->clientToVirtual(rcClient.getCenter());

			for (auto node : data->getNodes())
		 	{
		 		// Create new unique instance ID.
		 		node->setId(Guid::create());

		 		// Place node in view.
		 		//std::pair< int, int > position = node->getPosition();
		 		//position.first = center.x + position.first - bounds.left - bounds.getWidth() / 2;
		 		//position.second = center.y + position.second - bounds.top - bounds.getHeight() / 2;
		 		//node->setPosition(position);

		 		// Add node to graph.
		 		m_imageGraph->addNode(node);
		 	}

			for (auto edge : data->getEdges())
		 		m_imageGraph->addEdge(edge);

		 	createEditorGraph();
		 }
	}
	else if (command == L"Editor.SelectAll")
	{
		m_editorGraph->selectAllNodes();
		// updateGraph();
	}
	else if (command == L"Editor.Unselect")
	{
		m_editorGraph->deselectAllNodes();
		// updateGraph();
	}
	else if (command == L"Editor.Delete")
	{
		const RefArray< ui::Node > nodes = m_editorGraph->getSelectedNodes();
		if (nodes.empty())
			return false;

		// Save undo state.
		m_document->push();

		// Remove edges first which are connected to selected nodes.
		const RefArray< ui::Edge > edges = m_editorGraph->getConnectedEdges(nodes, false);
		for (auto editorEdge : edges)
		{
			Ref< Edge > edge = editorEdge->getData< Edge >(L"IMGEDGE");
			m_editorGraph->removeEdge(editorEdge);
			m_imageGraph->removeEdge(edge);
		}
		for (auto editorNode : nodes)
		{
			Ref< Node > shaderNode = editorNode->getData< Node >(L"IMGNODE");
			m_editorGraph->removeNode(editorNode);
			m_imageGraph->removeNode(shaderNode);
		}

		createEditorGraph();
	}
	else if (command == L"Editor.Undo")
	{
		if (m_document->undo())
		{
			m_imageGraph = m_document->getObject< ImageGraphAsset >(0);
			T_ASSERT(m_imageGraph);

			createEditorGraph();
		}
	}
	else if (command == L"Editor.Redo")
	{
		if (m_document->redo())
		{
			m_imageGraph = m_document->getObject< ImageGraphAsset >(0);
			T_ASSERT(m_imageGraph);

			createEditorGraph();
		}
	}
	else if (command == L"ImageGraph.Editor.Center")
	{
		m_editorGraph->center();
	}
	else if (command == L"ImageGraph.Editor.AlignLeft")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnLeft);
	}
	else if (command == L"ImageGraph.Editor.AlignRight")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnRight);
	}
	else if (command == L"ImageGraph.Editor.AlignTop")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnTop);
	}
	else if (command == L"ImageGraph.Editor.AlignBottom")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnBottom);
	}
	else if (command == L"ImageGraph.Editor.EvenSpaceVertically")
	{
		m_document->push();
		m_editorGraph->evenSpace(ui::GraphControl::EsVertically);
	}
	else if (command == L"ImageGraph.Editor.EvenSpaceHorizontally")
	{
		m_document->push();
		m_editorGraph->evenSpace(ui::GraphControl::EsHorizontally);
	}
	else if (command == L"ImageGraph.Editor.AddInput")
	{
		// Create image graph input.
		Ref< ImgInput > input = new ImgInput();
		input->setId(Guid::create());
		m_imageGraph->addNode(input);

		// Create node in graph control.
		createEditorNode(input);
	}
	else if (command == L"ImageGraph.Editor.AddOutput")
	{
		// Create image graph output.
		Ref< ImgOutput > output = new ImgOutput();
		output->setId(Guid::create());
		m_imageGraph->addNode(output);

		// Create node in graph control.
		createEditorNode(output);
	}
	else if (command == L"ImageGraph.Editor.AddPass")
	{
		// Create image graph pass.
		Ref< ImgPass > pass = new ImgPass();
		pass->setId(Guid::create());
		m_imageGraph->addNode(pass);

		// Create node in graph control.
		createEditorNode(pass);
	}
	else if (command == L"ImageGraph.Editor.AddTargetSet")
	{
		// Create image graph target set.
		Ref< ImgTargetSet > targetSet = new ImgTargetSet();
		targetSet->setId(Guid::create());
		m_imageGraph->addNode(targetSet);

		// Create node in graph control.
		createEditorNode(targetSet);
	}
	else if (command == L"ImageGraph.Editor.AddTexture")
	{
		// Create image graph texture reference.
		Ref< ImgTexture > texture = new ImgTexture();
		texture->setId(Guid::create());
		m_imageGraph->addNode(texture);

		// Create node in graph control.
		createEditorNode(texture);
	}
	else if (command == L"ImageGraph.Editor.AddStructBuffer")
	{
		// Create image graph struct buffer reference.
		Ref< ImgStructBuffer > sbuffer = new ImgStructBuffer();
		sbuffer->setId(Guid::create());
		m_imageGraph->addNode(sbuffer);

		// Create node in graph control.
		createEditorNode(sbuffer);
	}
	else
		return false;

	m_editorGraph->update();
	return true;
}

void ImageGraphEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

Ref< ui::Node > ImageGraphEditorPage::createEditorNode(Node* node) const
{
	Ref< ui::Node > editorNode;

	const std::pair< int, int >& p = node->getPosition();
	const ui::UnitPoint position(ui::Unit(p.first), ui::Unit(p.second));

	if (auto input = dynamic_type_cast< ImgInput* >(node))
	{
		editorNode = m_editorGraph->createNode(
			L"Input",
			input->getTextureId(),
			position,
			new ui::InputNodeShape()
		);
	}
	else if (auto output = dynamic_type_cast< ImgOutput* >(node))
	{
		editorNode = m_editorGraph->createNode(
			L"Output",
			L"",
			position,
			new ui::OutputNodeShape()
		);
	}
	else if (auto pass = dynamic_type_cast< ImgPass* >(node))
	{
		editorNode = m_editorGraph->createNode(
			L"Pass",
			pass->getName(),
			position,
			new ui::DefaultNodeShape(ui::DefaultNodeShape::StDefault)
		);
	}
	else if (auto targetSet = dynamic_type_cast< ImgTargetSet* >(node))
	{
		editorNode = m_editorGraph->createNode(
			L"TargetSet",
			targetSet->getTargetSetId(),
			position,
			new ui::DefaultNodeShape(ui::DefaultNodeShape::StExternal)
		);
	}
	else if (auto texture = dynamic_type_cast< ImgTexture* >(node))
	{
		Ref< db::Instance > textureInstance = m_editor->getSourceDatabase()->getInstance(texture->getTexture());
		editorNode = m_editorGraph->createNode(
			L"Texture",
			textureInstance ? textureInstance->getName() : Guid(texture->getTexture()).format(),
			position,
			new ui::InputNodeShape()
		);
	}
	else if (auto sbuffer = dynamic_type_cast< ImgStructBuffer* >(node))
	{
		editorNode = m_editorGraph->createNode(
			L"Struct buffer",
			L"",
			position,
			new ui::DefaultNodeShape(ui::DefaultNodeShape::StExternal)
		);
	}
	else
	{
		T_FATAL_ERROR;
		return nullptr;
	}

	for (int i = 0; i < node->getInputPinCount(); ++i)
	{
		const InputPin* inputPin = node->getInputPin(i);
		editorNode->createInputPin(inputPin->getName(), Guid(), false, false);
	}

	for (int i = 0; i < node->getOutputPinCount(); ++i)
	{
		const OutputPin* outputPin = node->getOutputPin(i);
		editorNode->createOutputPin(outputPin->getName(), Guid());
	}

	editorNode->setData(L"IMGNODE", node);
	return editorNode;
}

void ImageGraphEditorPage::createEditorGraph()
{
	// Keep a map from image graph nodes to editor nodes.
	std::map< Ref< Node >, Ref< ui::Node > > nodeMap;

	// Ensure editor graph is clean.
	m_editorGraph->removeAllEdges();
	m_editorGraph->removeAllNodes();

	// Create editor nodes for each image graph node.
	for (auto node : m_imageGraph->getNodes())
	{
		Ref< ui::Node > editorNode = createEditorNode(node);
		nodeMap[node] = editorNode;
	}

	// Create editor edges for each shader edge.
	for (auto edge : m_imageGraph->getEdges())
	{
		const OutputPin* sourcePin = edge->getSource();
		if (!sourcePin)
		{
			log::warning << L"Invalid edge, no source pin." << Endl;
			continue;
		}

		const InputPin* destinationPin = edge->getDestination();
		if (!destinationPin)
		{
			log::warning << L"Invalid edge, no destination pin." << Endl;
			continue;
		}

		ui::Node* editorSourceNode = nodeMap[sourcePin->getNode()];
		if (!editorSourceNode)
		{
			log::warning << L"Invalid pin, no editor source node found of pin \"" << sourcePin->getName() << L"\"." << Endl;
			continue;
		}

		ui::Node* editorDestinationNode = nodeMap[destinationPin->getNode()];
		if (!editorDestinationNode)
		{
			log::warning << L"Invalid pin, no editor destination node found of pin \"" << destinationPin->getName() << L"\"." << Endl;
			continue;
		}

		ui::Pin* editorSourcePin = editorSourceNode->findOutputPin(sourcePin->getName());
		if (!editorSourcePin)
		{
			log::warning << L"Unable to find editor source pin \"" << sourcePin->getName() << L"\"." << Endl;
			continue;
		}

		ui::Pin* editorDestinationPin = editorDestinationNode->findInputPin(destinationPin->getName());
		if (!editorDestinationPin)
		{
			log::warning << L"Unable to find editor destination pin \"" << destinationPin->getName() << L"\"." << Endl;
			continue;
		}

		Ref< ui::Edge > editorEdge = new ui::Edge(editorSourcePin, editorDestinationPin);
		editorEdge->setData(L"IMGEDGE", edge);

		m_editorGraph->addEdge(editorEdge);
	}

	m_editorGraph->update();
}

void ImageGraphEditorPage::eventToolClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	handleCommand(command);
}

void ImageGraphEditorPage::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtRight)
		return;

	const ui::MenuItem* selected = m_menuPopup->showModal(m_editorGraph, event->getPosition());
	if (!selected)
		return;

	const ui::Command& command = selected->getCommand();
	handleCommand(command);
}

void ImageGraphEditorPage::eventSelect(ui::SelectEvent* event)
{
	const RefArray< ui::Node > nodes = m_editorGraph->getSelectedNodes();
	if (nodes.size() == 1)
	{
		Node* node = nodes.front()->getData< Node >(L"IMGNODE");
		if (node)
		{
			// Create a clone of the selected node as we cannot
			// allow in-place editing of a node as graph contain
			// pointers to pins within each node.
			m_propertiesNode = DeepClone(node).create< Node >();
			m_propertiesView->setPropertyObject(m_propertiesNode);
			return;
		}
	}

	// Multiple nodes or no node selected; do not show
	// any properties in any case.
	m_propertiesView->setPropertyObject(nullptr);
	m_propertiesNode = nullptr;
}

void ImageGraphEditorPage::eventNodeMoved(ui::NodeMovedEvent* event)
{
	const ui::Node* editorNode = event->getNode();
	Node* node = editorNode->getData< Node >(L"IMGNODE");

	const ui::UnitPoint position = editorNode->getPosition();

	// Save position in node.
	node->setPosition(std::make_pair(
		position.x.get(),
		position.y.get()
	));
}

void ImageGraphEditorPage::eventEdgeConnect(ui::EdgeConnectEvent* event)
{
	Ref< ui::Edge > editorEdge = event->getEdge();
	Ref< ui::Pin > editorSourcePin = editorEdge->getSourcePin();
	T_ASSERT(editorSourcePin);

	ui::Pin* editorDestinationPin = editorEdge->getDestinationPin();
	T_ASSERT(editorDestinationPin);

	Node* sourceNode = editorSourcePin->getNode()->getData< Node >(L"IMGNODE");
	T_ASSERT(sourceNode);

	Node* destinationNode = editorDestinationPin->getNode()->getData< Node >(L"IMGNODE");
	T_ASSERT(destinationNode);

	// Ensure compatible types of nodes are connected.
	const bool sourceTarget = is_a< ImgInput >(sourceNode) || is_a< ImgOutput >(sourceNode) || is_a< ImgStructBuffer >(sourceNode) || is_a< ImgTargetSet >(sourceNode) || is_a< ImgTexture >(sourceNode);
	const bool destinationTarget = is_a< ImgInput >(destinationNode) || is_a< ImgOutput >(destinationNode) || is_a< ImgStructBuffer >(destinationNode) || is_a< ImgTargetSet >(destinationNode);
	if (sourceTarget == destinationTarget)
	{
		log::warning << L"Only \"pass to target\" or \"target to pass\" can be connected." << Endl;
		return;
	}

	const OutputPin* sourcePin = sourceNode->findOutputPin(editorSourcePin->getName());
	T_ASSERT(sourcePin);

	const InputPin* destinationPin = destinationNode->findInputPin(editorDestinationPin->getName());
	T_ASSERT(destinationPin);

	// Replace existing edge.
	Ref< Edge > edge = m_imageGraph->findEdge(destinationPin);
	if (edge)
	{
		m_imageGraph->removeEdge(edge);

		RefArray< ui::Edge > editorEdges = m_editorGraph->getConnectedEdges(editorDestinationPin);
		if (editorEdges.size() >= 1)
			m_editorGraph->removeEdge(editorEdges.front());
	}

	m_document->push();

	edge = new Edge(sourcePin, destinationPin);
	m_imageGraph->addEdge(edge);

	editorEdge->setData(L"IMGEDGE", edge);
	m_editorGraph->addEdge(editorEdge);
}

void ImageGraphEditorPage::eventEdgeDisconnect(ui::EdgeDisconnectEvent* event)
{
	ui::Edge* editorEdge = event->getEdge();
	Edge* edge = mandatory_non_null_type_cast< Edge* >(editorEdge->getData(L"IMGEDGE"));

	m_document->push();
	m_imageGraph->removeEdge(edge);
}

void ImageGraphEditorPage::eventPropertiesChanging(ui::ContentChangingEvent* event)
{
	m_document->push();
}

void ImageGraphEditorPage::eventPropertiesChanged(ui::ContentChangeEvent* event)
{
	const RefArray< ui::Node > nodes = m_editorGraph->getSelectedNodes();
	if (nodes.size() == 1 && m_propertiesNode != nullptr)
	{
		Node* node = nodes.front()->getData< Node >(L"IMGNODE");
		T_FATAL_ASSERT(&type_of(m_propertiesNode) == &type_of(node));

		// Keep position; user might have moved node while being selected.
		m_propertiesNode->setPosition(node->getPosition());

		m_imageGraph->replace(
			node,
			m_propertiesNode
		);

		createEditorGraph();

		// Find and select re-created node.
		for (auto n : m_editorGraph->getNodes())
		{
			if (n->getData< Node >(L"IMGNODE") == m_propertiesNode)
				n->setSelected(true);
		}

		// Re-create a clone of the selected node as we cannot
		// allow in-place editing of a node as graph contain
		// pointers to pins within each node.
		m_propertiesNode = DeepClone(m_propertiesNode).create< Node >();
		m_propertiesView->setPropertyObject(m_propertiesNode);
	}
}

}
