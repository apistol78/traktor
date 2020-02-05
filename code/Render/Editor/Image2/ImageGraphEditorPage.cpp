#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Editor/IDocument.h"
#include "Editor/IEditorPageSite.h"
#include "Render/Editor/Image2/IgaPass.h"
#include "Render/Editor/Image2/IgaTarget.h"
#include "Render/Editor/Image2/ImageGraphAsset.h"
#include "Render/Editor/Image2/ImageGraphEditorPage.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/TableLayout.h"
#include "Ui/Graph/DefaultNodeShape.h"
#include "Ui/Graph/Edge.h"
#include "Ui/Graph/EdgeConnectEvent.h"
#include "Ui/Graph/EdgeDisconnectEvent.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/NodeMovedEvent.h"
#include "Ui/Graph/Pin.h"
#include "Ui/Graph/SelectEvent.h"

namespace traktor
{
    namespace render
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
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0, 0));

	m_editorGraph = new ui::GraphControl();
	m_editorGraph->create(container);
    m_editorGraph->addEventHandler< ui::MouseButtonDownEvent >(this, &ImageGraphEditorPage::eventButtonDown);
	m_editorGraph->addEventHandler< ui::SelectEvent >(this, &ImageGraphEditorPage::eventSelect);
	m_editorGraph->addEventHandler< ui::NodeMovedEvent >(this, &ImageGraphEditorPage::eventNodeMoved);
	//m_editorGraph->addEventHandler< ui::NodeActivateEvent >(this, &ImageGraphEditorPage::eventNodeDoubleClick);
	m_editorGraph->addEventHandler< ui::EdgeConnectEvent >(this, &ImageGraphEditorPage::eventEdgeConnect);
	m_editorGraph->addEventHandler< ui::EdgeDisconnectEvent >(this, &ImageGraphEditorPage::eventEdgeDisconnect);

	m_menuPopup = new ui::Menu();
	Ref< ui::MenuItem > menuItemCreate = new ui::MenuItem(L"Create...");
    menuItemCreate->add(new ui::MenuItem(ui::Command(L"ImageGraph.Editor.AddPass"), L"Pass"));
    menuItemCreate->add(new ui::MenuItem(ui::Command(L"ImageGraph.Editor.AddTarget"), L"Target"));
    m_menuPopup->add(menuItemCreate);
    m_menuPopup->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), L"Delete"));

    createEditorGraph();
	return true;
}

void ImageGraphEditorPage::destroy()
{
	safeDestroy(m_editorGraph);
}

bool ImageGraphEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool ImageGraphEditorPage::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.PropertiesChanging")
	{
		m_document->push();
	}
	else if (command == L"Editor.PropertiesChanged")
	{
		// refreshGraph();
		// updateGraph();
	}
	else if (command == L"Editor.Cut" || command == L"Editor.Copy")
	{
		// RefArray< ui::Node > selectedNodes;
		// if (m_editorGraph->getSelectedNodes(selectedNodes) > 0)
		// {
		// 	// Also copy edges which are affected by selected nodes.
		// 	RefArray< ui::Edge > selectedEdges;
		// 	m_editorGraph->getConnectedEdges(selectedNodes, true, selectedEdges);

		// 	Ref< ImageGraphEditorClipboardData > data = new ImageGraphEditorClipboardData();

		// 	ui::Rect bounds(0, 0, 0, 0);
		// 	for (auto i = selectedNodes.begin(); i != selectedNodes.end(); ++i)
		// 	{
		// 		Ref< Node > shaderNode = (*i)->getData< Node >(L"SHADERNODE");
		// 		T_ASSERT(shaderNode);
		// 		data->addNode(shaderNode);

		// 		if (i != selectedNodes.begin())
		// 		{
		// 			ui::Rect rc = (*i)->calculateRect();
		// 			bounds.left = std::min(bounds.left, rc.left);
		// 			bounds.top = std::min(bounds.top, rc.top);
		// 			bounds.right = std::max(bounds.right, rc.right);
		// 			bounds.bottom = std::max(bounds.bottom, rc.bottom);
		// 		}
		// 		else
		// 			bounds = (*i)->calculateRect();
		// 	}

		// 	data->setBounds(bounds);

		// 	for (auto selectedEdge : selectedEdges)
		// 	{
		// 		Ref< Edge > shaderEdge = selectedEdge->getData< Edge >(L"SHADEREDGE");
		// 		T_ASSERT(shaderEdge);
		// 		data->addEdge(shaderEdge);
		// 	}

		// 	ui::Application::getInstance()->getClipboard()->setObject(data);

		// 	// Remove edges and nodes from graphs if user cuts.
		// 	if (command == L"Editor.Cut")
		// 	{
		// 		// Save undo state.
		// 		m_document->push();

		// 		// Remove edges which are connected to any selected node, not only those who connects to both selected end nodes.
		// 		selectedEdges.resize(0);
		// 		m_editorGraph->getConnectedEdges(selectedNodes, false, selectedEdges);

		// 		for (auto selectedEdge : selectedEdges)
		// 		{
		// 			m_imageGraph->removeEdge(selectedEdge->getData< Edge >(L"SHADEREDGE"));
		// 			m_editorGraph->removeEdge(selectedEdge);
		// 		}

		// 		for (auto selectedNode : selectedNodes)
		// 		{
		// 			m_imageGraph->removeNode(selectedNode->getData< Node >(L"SHADERNODE"));
		// 			m_editorGraph->removeNode(selectedNode);
		// 		}
		// 	}
		// }
	}
	else if (command == L"Editor.Paste")
	{
		// Ref< ImageGraphEditorClipboardData > data = dynamic_type_cast< ImageGraphEditorClipboardData* >(
		// 	ui::Application::getInstance()->getClipboard()->getObject()
		// );
		// if (data)
		// {
		// 	// Save undo state.
		// 	m_document->push();

		// 	const ui::Rect& bounds = data->getBounds();

		// 	ui::Rect rcClient = m_editorGraph->getInnerRect();
		// 	ui::Point center = m_editorGraph->clientToVirtual(rcClient.getCenter());

		// 	for (RefArray< Node >::const_iterator i = data->getNodes().begin(); i != data->getNodes().end(); ++i)
		// 	{
		// 		// Create new unique instance ID.
		// 		(*i)->setId(Guid::create());

		// 		// Place node in view.
		// 		std::pair< int, int > position = (*i)->getPosition();
		// 		position.first = ui::invdpi96(center.x + ui::dpi96(position.first) - bounds.left - bounds.getWidth() / 2);
		// 		position.second = ui::invdpi96(center.y + ui::dpi96(position.second) - bounds.top - bounds.getHeight() / 2);
		// 		(*i)->setPosition(position);

		// 		// Add node to graph.
		// 		m_imageGraph->addNode(*i);
		// 	}

		// 	for (RefArray< Edge >::const_iterator i = data->getEdges().begin(); i != data->getEdges().end(); ++i)
		// 		m_imageGraph->addEdge(*i);

		// 	createEditorNodes(
		// 		data->getNodes(),
		// 		data->getEdges()
		// 	);
		// 	updateGraph();
		// }
	}
	else if (command == L"Editor.SelectAll")
	{
		m_editorGraph->selectAllNodes();
		// updateGraph();
	}
	else if (command == L"Editor.Delete")
	{
		// RefArray< ui::Node > nodes;
		// if (m_editorGraph->getSelectedNodes(nodes) <= 0)
		// 	return false;

		// // Save undo state.
		// m_document->push();

		// // Remove edges first which are connected to selected nodes.
		// RefArray< ui::Edge > edges;
		// m_editorGraph->getConnectedEdges(nodes, false, edges);

		// for (RefArray< ui::Edge >::iterator i = edges.begin(); i != edges.end(); ++i)
		// {
		// 	ui::Edge* editorEdge = *i;
		// 	Ref< Edge > shaderEdge = editorEdge->getData< Edge >(L"SHADEREDGE");

		// 	m_editorGraph->removeEdge(editorEdge);
		// 	m_imageGraph->removeEdge(shaderEdge);
		// }

		// for (RefArray< ui::Node >::iterator i = nodes.begin(); i != nodes.end(); ++i)
		// {
		// 	ui::Node* editorNode = *i;
		// 	Ref< Node > shaderNode = editorNode->getData< Node >(L"SHADERNODE");

		// 	m_editorGraph->removeNode(editorNode);
		// 	m_imageGraph->removeNode(shaderNode);
		// }

		// updateGraph();
	}
	else if (command == L"Editor.Undo")
	{
		if (m_document->undo())
		{
			m_imageGraph = m_document->getObject< ImageGraphAsset >(0);
			T_ASSERT(m_imageGraph);

			// createEditorGraph();
		}
	}
	else if (command == L"Editor.Redo")
	{
		if (m_document->redo())
		{
			m_imageGraph = m_document->getObject< ImageGraphAsset >(0);
			T_ASSERT(m_imageGraph);

			// createEditorGraph();
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
    else if (command == L"ImageGraph.Editor.AddPass")
    {
		// Create image graph pass.
		Ref< IgaPass > pass = new IgaPass(L"Unnamed");
		m_imageGraph->addPass(pass);

		// Create node in graph control.
		Ref< ui::Node > passNode = new ui::Node(
			pass->getName(),
			L"Pass",
			ui::Point(0, 0),
			new ui::DefaultNodeShape(m_editorGraph, ui::DefaultNodeShape::StExternal)
		);
		passNode->createInputPin(L"Input", false);
		passNode->createOutputPin(L"Output");
		passNode->setData(L"PASS", pass);
		m_editorGraph->addNode(passNode);
    }
    else if (command == L"ImageGraph.Editor.AddTarget")
    {
		// Create image graph target.
		Ref< IgaTarget > target = new IgaTarget(L"Unnamed");
		m_imageGraph->addTarget(target);

		// Create node in graph control.
		Ref< ui::Node > targetNode = new ui::Node(
			target->getName(),
			L"Target",
			ui::Point(0, 0),
			new ui::DefaultNodeShape(m_editorGraph, ui::DefaultNodeShape::StDefault)
		);
		targetNode->createInputPin(L"Input", false);
		targetNode->createOutputPin(L"Output");
		targetNode->setData(L"TARGET", target);
        m_editorGraph->addNode(targetNode);
    }
	else
		return false;

	m_editorGraph->update();
	return true;
}

void ImageGraphEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void ImageGraphEditorPage::createEditorGraph()
{
    // Add targets defined in asset.
    for (auto target : m_imageGraph->getTargets())
    {
		const int32_t* p = target->getPosition();

		Ref< ui::Node > targetNode = new ui::Node(
			target->getName(),
			L"Target",
			ui::Point(ui::dpi96(p[0]), ui::dpi96(p[1])),
			new ui::DefaultNodeShape(m_editorGraph, ui::DefaultNodeShape::StDefault)
		);
		targetNode->createInputPin(L"Input", false);
		targetNode->createOutputPin(L"Output");
		targetNode->setData(L"TARGET", target);
        m_editorGraph->addNode(targetNode);
    }

    // Add passes defined in asset.
    for (auto pass : m_imageGraph->getPasses())
    {
		const int32_t* p = pass->getPosition();

		Ref< ui::Node > passNode = new ui::Node(
			pass->getName(),
			L"Pass",
			ui::Point(ui::dpi96(p[0]), ui::dpi96(p[1])),
			new ui::DefaultNodeShape(m_editorGraph, ui::DefaultNodeShape::StExternal)
		);
		passNode->setData(L"PASS", pass);

		// Create input pins and input edges.
		for (const auto& input : pass->getInputs())
		{
			ui::Pin* inputPin = passNode->createInputPin(input.name, false);
			if (input.source != nullptr)
			{
				for (auto editorNode : m_editorGraph->getNodes())
				{
					if (editorNode->getData< IgaTarget >(L"TARGET") == input.source)
					{
						m_editorGraph->addEdge(new ui::Edge(
							editorNode->getOutputPin(0),
							inputPin
						));
						break;
					}
				}
			}
		}

		// Create output pin and output edge.
		{
			const auto& output = pass->getOutput();

			ui::Pin* outputPin = passNode->createOutputPin(L"Output");
			if (output.target != nullptr)
			{
				for (auto editorNode : m_editorGraph->getNodes())
				{
					if (editorNode->getData< IgaTarget >(L"TARGET") == output.target)
					{
						m_editorGraph->addEdge(new ui::Edge(
							outputPin,
							editorNode->getInputPin(0)
						));
						break;
					}
				}
			}
		}
	
		// Add pass node to graph.
        m_editorGraph->addNode(passNode);
	}

	m_editorGraph->center();
    m_editorGraph->update();
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
	RefArray< ui::Node > nodes;
	if (m_editorGraph->getSelectedNodes(nodes) == 1)
	{
		IgaPass* pass = nodes[0]->getData< IgaPass >(L"PASS");
		if (pass)
		{
			m_site->setPropertyObject(pass);
			return;
		}

		IgaTarget* target = nodes[0]->getData< IgaTarget >(L"TARGET");
		if (target)
		{
			m_site->setPropertyObject(target);
			return;
		}
	}
	m_site->setPropertyObject(nullptr);
}

void ImageGraphEditorPage::eventNodeMoved(ui::NodeMovedEvent* event)
{
	ui::Node* node = event->getNode();

	// Get dpi agnostic position.
	ui::Point position = node->getPosition();
	position.x = ui::invdpi96(position.x);
	position.y = ui::invdpi96(position.y);

	// Save position in pass or target.
	IgaPass* pass = node->getData< IgaPass >(L"PASS");
	if (pass)
	{
		pass->setPosition(
			position.x,
			position.y
		);
	}

	IgaTarget* target = node->getData< IgaTarget >(L"TARGET");
	if (target)
	{
		target->setPosition(
			position.x,
			position.y
		);
	}
}

void ImageGraphEditorPage::eventEdgeConnect(ui::EdgeConnectEvent* event)
{
	ui::Edge* edge = event->getEdge();
	ui::Pin* sourcePin = edge->getSourcePin();
	ui::Pin* destinationPin = edge->getDestinationPin();

	m_document->push();

	bool attached = false;

	// "Render target" to "Pass"
	{
		IgaTarget* source = sourcePin->getNode()->getData< IgaTarget >(L"TARGET");
		IgaPass* destination = destinationPin->getNode()->getData< IgaPass >(L"PASS");
		if (source != nullptr && destination != nullptr)
			attached |= destination->attachInput(destinationPin->getName(), source);
	}

	// "Pass" to "Render target"
	{
		IgaPass* source = sourcePin->getNode()->getData< IgaPass >(L"PASS");
		IgaTarget* destination = destinationPin->getNode()->getData< IgaTarget >(L"TARGET");
		if (source != nullptr && destination != nullptr)
			attached |= source->attachOutput(destination);
	}

	// If nothing attached ignore adding this edge.
	if (!attached)
		return;

	// Disconnect existing edge first.
	RefArray< ui::Edge > edges;
	m_editorGraph->getConnectedEdges(destinationPin, edges);
	for (auto edge : edges)
		m_editorGraph->removeEdge(edge);

	// Create new edge.
	m_editorGraph->addEdge(edge);
}

void ImageGraphEditorPage::eventEdgeDisconnect(ui::EdgeDisconnectEvent* event)
{
	ui::Edge* edge = event->getEdge();
	ui::Pin* sourcePin = edge->getSourcePin();
	ui::Pin* destinationPin = edge->getDestinationPin();

	m_document->push();

	bool detached = false;

	{
		IgaPass* pass = sourcePin->getNode()->getData< IgaPass >(L"PASS");
		if (pass)
			detached |= pass->attachOutput(nullptr);
	}

	{
		IgaPass* pass = destinationPin->getNode()->getData< IgaPass >(L"PASS");
		if (pass)
			detached |= pass->attachInput(destinationPin->getName(), nullptr);
	}

	if (!detached)
		log::warning << L"No input/output detached from disconnecting edge." << Endl;
}

    }
}
