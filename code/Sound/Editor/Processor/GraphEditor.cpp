/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Serialization/DeepClone.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Editor/PropertiesView.h"
#include "I18N/Text.h"
#include "Resource/ResourceManager.h"
#include "Sound/AudioChannel.h"
#include "Sound/AudioSystem.h"
#include "Sound/SoundFactory.h"
#include "Sound/Editor/WaveformControl.h"
#include "Sound/Editor/Processor/GraphAsset.h"
#include "Sound/Editor/Processor/GraphEditor.h"
#include "Sound/Processor/Edge.h"
#include "Sound/Processor/Graph.h"
#include "Sound/Processor/GraphBuffer.h"
#include "Sound/Processor/InputPin.h"
#include "Sound/Processor/Node.h"
#include "Sound/Processor/OutputPin.h"
#include "Sound/Processor/Nodes/Scalar.h"
#include "Sound/Processor/Nodes/Output.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/Splitter.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Graph/DefaultNodeShape.h"
#include "Ui/Graph/Edge.h"
#include "Ui/Graph/EdgeConnectEvent.h"
#include "Ui/Graph/EdgeDisconnectEvent.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/InputNodeShape.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/NodeActivateEvent.h"
#include "Ui/Graph/NodeMovedEvent.h"
#include "Ui/Graph/OutputNodeShape.h"
#include "Ui/Graph/Pin.h"
#include "Ui/Graph/SelectEvent.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarSeparator.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

class NodeType : public Object
{
	T_RTTI_CLASS;

public:
	explicit NodeType(const TypeInfo* type)
	:	m_type(type)
	{
	}

	const TypeInfo* getNodeType() const
	{
		return m_type;
	}

private:
	const TypeInfo* m_type;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.GraphEditor.NodeType", NodeType, Object)

std::wstring getLocalizedName(const TypeInfo* nodeType)
{
	std::wstring nodeName = nodeType->getName();
	size_t p = nodeName.find_last_of(L'.');
	return i18n::Text(L"SOUND_PROCESSOR_NODE_" + toUpper(nodeName.substr(p + 1)));
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.GraphEditor", GraphEditor, editor::IEditorPage)

GraphEditor::GraphEditor(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
{
}

bool GraphEditor::create(ui::Container* parent)
{
	m_graphAsset = m_document->getObject< GraphAsset >(0);
	if (!m_graphAsset)
		return false;

	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	m_toolBarGraph = new ui::ToolBar();
	m_toolBarGraph->create(container);
	m_toolBarGraph->addImage(new ui::StyleBitmap(L"Input.Alignment"), 6);
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"SOUND_PROCESSOR_EDITOR_ALIGN_LEFT"), 0, ui::Command(L"Sound.Processor.Editor.AlignLeft")));
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"SOUND_PROCESSOR_EDITOR_ALIGN_RIGHT"), 1, ui::Command(L"Sound.Processor.Editor.AlignRight")));
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"SOUND_PROCESSOR_EDITOR_ALIGN_TOP"), 2, ui::Command(L"Sound.Processor.Editor.AlignTop")));
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"SOUND_PROCESSOR_EDITOR_ALIGN_BOTTOM"), 3, ui::Command(L"Sound.Processor.Editor.AlignBottom")));
	m_toolBarGraph->addItem(new ui::ToolBarSeparator());
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"SOUND_PROCESSOR_EDITOR_EVEN_VERTICALLY"), 4, ui::Command(L"Sound.Processor.Editor.EvenSpaceVertically")));
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"SOUND_PROCESSOR_EDITOR_EVEN_HORIZONTALLY"), 5, ui::Command(L"Sound.Processor.Editor.EventSpaceHorizontally")));
	m_toolBarGraph->addEventHandler< ui::ToolBarButtonClickEvent >(this, &GraphEditor::eventToolBarGraphClick);

	m_graph = new ui::GraphControl();
	m_graph->create(container);
	m_graph->setText(L"SOUND");
	m_graph->addEventHandler< ui::MouseButtonDownEvent >(this, &GraphEditor::eventButtonDown);
	m_graph->addEventHandler< ui::SelectEvent >(this, &GraphEditor::eventNodeSelect);
	m_graph->addEventHandler< ui::NodeMovedEvent >(this, &GraphEditor::eventNodeMoved);
	m_graph->addEventHandler< ui::NodeActivateEvent >(this, &GraphEditor::eventNodeActivated);
	m_graph->addEventHandler< ui::EdgeConnectEvent >(this, &GraphEditor::eventEdgeConnected);
	m_graph->addEventHandler< ui::EdgeDisconnectEvent >(this, &GraphEditor::eventEdgeDisconnected);

	// Create properties view.
	m_propertiesView = m_site->createPropertiesView(parent);
	m_propertiesView->addEventHandler< ui::ContentChangingEvent >(this, &GraphEditor::eventPropertiesChanging);
	m_propertiesView->addEventHandler< ui::ContentChangeEvent >(this, &GraphEditor::eventPropertiesChanged);
	m_site->createAdditionalPanel(m_propertiesView, ui::dpi96(400), false);

	// Build popup menu.
	m_menuPopup = new ui::Menu();

	Ref< ui::MenuItem > menuItemCreate = new ui::MenuItem(i18n::Text(L"SOUND_PROCESSOR_EDITOR_CREATE_NODE"));
	for (auto nodeType : type_of< Node >().findAllOf(false))
	{
		if (!nodeType->isInstantiable())
			continue;

		Ref< ui::MenuItem > menuItem = new ui::MenuItem(ui::Command(L"Sound.Processor.Editor.Create"), getLocalizedName(nodeType));
		menuItem->setData(L"TYPE", new NodeType(nodeType));
		menuItemCreate->add(menuItem);
	}

	m_menuPopup->add(menuItemCreate);
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"SOUND_PROCESSOR_EDITOR_DELETE_NODE")));

	// Get audio system for preview.
	m_audioSystem = m_editor->getStoreObject< AudioSystem >(L"AudioSystem");
	if (m_audioSystem)
	{
		m_audioChannel = m_audioSystem->getChannel(0);
		if (!m_audioChannel)
			m_audioSystem = nullptr;
	}
	if (!m_audioSystem)
		log::warning << L"Unable to create preview audio system; preview unavailable" << Endl;

	m_resourceManager = new resource::ResourceManager(m_editor->getOutputDatabase(), m_editor->getSettings()->getProperty< bool >(L"Resource.Verbose", false));
	m_resourceManager->addFactory(new SoundFactory());

	updateView();
	return true;
}

void GraphEditor::destroy()
{
	if (m_audioChannel)
	{
		m_audioChannel->stop();
		m_audioChannel = nullptr;
	}

	if (m_resourceManager)
		m_resourceManager = nullptr;

	m_audioSystem = nullptr;

	m_site->destroyAdditionalPanel(m_propertiesView);

	safeDestroy(m_propertiesView);
}

bool GraphEditor::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool GraphEditor::handleCommand(const ui::Command& command)
{
	if (m_propertiesView->handleCommand(command))
		return true;
	
	if (command == L"Editor.Undo")
	{
		if (m_document->undo())
		{
			m_graphAsset = m_document->getObject< GraphAsset >(0);
			T_FATAL_ASSERT (m_graphAsset);

			updateView();

			m_propertiesView->setPropertyObject(nullptr);
		}
	}
	else if (command == L"Editor.Redo")
	{
		if (m_document->redo())
		{
			m_graphAsset = m_document->getObject< GraphAsset >(0);
			T_FATAL_ASSERT (m_graphAsset);

			updateView();

			m_propertiesView->setPropertyObject(nullptr);
		}
	}
	else if (command == L"Sound.Processor.Editor.AlignLeft")
	{
		m_document->push();
		m_graph->alignNodes(ui::GraphControl::AnLeft);
		m_graph->update();
	}
	else if (command == L"Sound.Processor.Editor.AlignRight")
	{
		m_document->push();
		m_graph->alignNodes(ui::GraphControl::AnRight);
		m_graph->update();
	}
	else if (command == L"Sound.Processor.Editor.AlignTop")
	{
		m_document->push();
		m_graph->alignNodes(ui::GraphControl::AnTop);
		m_graph->update();
	}
	else if (command == L"Sound.Processor.Editor.AlignBottom")
	{
		m_document->push();
		m_graph->alignNodes(ui::GraphControl::AnBottom);
		m_graph->update();
	}
	else if (command == L"Sound.Processor.Editor.EvenSpaceVertically")
	{
		m_document->push();
		m_graph->evenSpace(ui::GraphControl::EsVertically);
		m_graph->update();
	}
	else if (command == L"Sound.Processor.Editor.EventSpaceHorizontally")
	{
		m_document->push();
		m_graph->evenSpace(ui::GraphControl::EsHorizontally);
		m_graph->update();
	}
	else
		return false;

	return true;
}

void GraphEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void GraphEditor::updateView()
{
	m_graph->removeAllEdges();
	m_graph->removeAllNodes();

	std::map< const InputPin*, ui::Pin* > inputPinMap;
	std::map< const OutputPin*, ui::Pin* > outputPinMap;

	const auto& nodes = m_graphAsset->getGraph()->getNodes();
	for (const auto node : nodes)
	{
		const std::pair< int, int >& position = node->getPosition();

		Ref< ui::Node > un;
		if (Scalar* scalar = dynamic_type_cast< Scalar* >(node))
		{
			un = m_graph->createNode(
				L"Scalar",
				toString(scalar->getValue()),
				ui::Point(position.first, position.second),
				new ui::InputNodeShape()
			);
		}
		else if (Output* output = dynamic_type_cast< Output* >(node))
		{
			un = m_graph->createNode(
				L"Output",
				L"",
				ui::Point(position.first, position.second),
				new ui::OutputNodeShape()
			);
		}
		else
		{
			un = m_graph->createNode(
				getLocalizedName(&type_of(node)),
				L"",
				ui::Point(position.first, position.second),
				new ui::DefaultNodeShape(ui::DefaultNodeShape::StDefault)
			);
		}

		size_t ipc = node->getInputPinCount();
		for (size_t i = 0; i < ipc; ++i)
		{
			const InputPin* ip = node->getInputPin(i);
			T_ASSERT(ip != nullptr);

			Ref< ui::Pin > up = un->createInputPin(ip->getName(), !ip->isOptional());
			up->setData(L"PIN", const_cast< InputPin* >(ip));

			inputPinMap[ip] = up;
		}

		size_t opc = node->getOutputPinCount();
		for (size_t i = 0; i < opc; ++i)
		{
			const OutputPin* op = node->getOutputPin(i);
			T_ASSERT(op != nullptr);

			Ref< ui::Pin > up = un->createOutputPin(op->getName());
			up->setData(L"PIN", const_cast< OutputPin* >(op));

			outputPinMap[op] = up;
		}

		un->setData(L"NODE", node);
	}

	const auto& edges = m_graphAsset->getGraph()->getEdges();
	for (const auto edge : edges)
	{
		ui::Pin* usp = outputPinMap[edge->getSource()];
		ui::Pin* udp = inputPinMap[edge->getDestination()];

		if (usp != nullptr && udp != nullptr)
		{
			Ref< ui::Edge > ue = new ui::Edge(
				usp,
				udp
			);
			ue->setData(L"EDGE", edge);
			m_graph->addEdge(ue);
		}
	}

	m_graph->update();
}

void GraphEditor::eventToolBarGraphClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	handleCommand(command);
}

void GraphEditor::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtRight)
		return;

	const ui::MenuItem* selected = m_menuPopup->showModal(m_graph, event->getPosition());
	if (!selected)
		return;

	if (selected->getCommand() == L"Sound.Processor.Editor.Create")
	{
		m_document->push();

		const NodeType* nt = selected->getData< NodeType >(L"TYPE");
		T_ASSERT(nt != nullptr);

		Ref< Node > node = dynamic_type_cast< Node* >(nt->getNodeType()->createInstance());
		T_ASSERT(node != nullptr);

		node->setPosition(std::make_pair(
			event->getPosition().x - m_graph->getOffset().cx,
			event->getPosition().y - m_graph->getOffset().cy
		));

		m_graphAsset->getGraph()->addNode(node);

		updateView();
	}
	else if (selected->getCommand() == L"Editor.Delete")
	{
		m_document->push();

		const RefArray< ui::Node > selectedNodes = m_graph->getSelectedNodes();
		const RefArray< ui::Edge > selectedEdges = m_graph->getConnectedEdges(selectedNodes, false);

		for (auto editorEdge : selectedEdges)
		{
			Ref< Edge > edge = editorEdge->getData< Edge >(L"EDGE");
			m_graph->removeEdge(editorEdge);
			m_graphAsset->getGraph()->removeEdge(edge);
		}

		for (auto editorNode : selectedNodes)
		{
			Ref< Node > node = editorNode->getData< Node >(L"NODE");
			m_graph->removeNode(editorNode);
			m_graphAsset->getGraph()->removeNode(node);
		}

		m_graph->update();
	}
}

void GraphEditor::eventNodeSelect(ui::SelectEvent* event)
{
	const RefArray< ui::Node >& nodes = event->getNodes();
	if (nodes.size() == 1)
		m_propertiesView->setPropertyObject(nodes[0]->getData< ISerializable >(L"NODE"));
	else
		m_propertiesView->setPropertyObject(nullptr);
}

void GraphEditor::eventNodeMoved(ui::NodeMovedEvent* event)
{
	ui::Node* un = event->getNode();
	T_ASSERT(un != nullptr);

	Node* node = un->getData< Node >(L"NODE");
	T_ASSERT(node != nullptr);

	node->setPosition(std::make_pair(
		un->getPosition().x,
		un->getPosition().y
	));
}

void GraphEditor::eventNodeActivated(ui::NodeActivateEvent* event)
{
}

void GraphEditor::eventEdgeConnected(ui::EdgeConnectEvent* event)
{
	ui::Edge* ue = event->getEdge();
	ui::Pin* usp = ue->getSourcePin();
	ui::Pin* udp = ue->getDestinationPin();

	OutputPin* sourcePin = usp->getData< OutputPin >(L"PIN");
	T_ASSERT(sourcePin != nullptr);

	InputPin* destinationPin = udp->getData< InputPin >(L"PIN");
	T_ASSERT(destinationPin != nullptr);

	Edge* currentEdge = m_graphAsset->getGraph()->findEdge(destinationPin);
	if (currentEdge != nullptr)
		m_graphAsset->getGraph()->removeEdge(currentEdge);

	Ref< Edge > edge = new Edge(
		sourcePin,
		destinationPin
	);
	m_graphAsset->getGraph()->addEdge(edge);

	ue->setData(L"EDGE", edge);

	// Accept connected edge by adding to control.
	m_graph->addEdge(ue);
	m_graph->update();
}

void GraphEditor::eventEdgeDisconnected(ui::EdgeDisconnectEvent* event)
{
	ui::Edge* ue = event->getEdge();

	Edge* edge = ue->getData< Edge >(L"EDGE");
	T_ASSERT(edge != nullptr);

	m_graphAsset->getGraph()->removeEdge(edge);

	// Accept removed edge by removing from control.
	m_graph->removeEdge(ue);
	m_graph->update();
}

void GraphEditor::eventPropertiesChanging(ui::ContentChangingEvent* event)
{
	m_document->push();
}

void GraphEditor::eventPropertiesChanged(ui::ContentChangeEvent* event)
{
	updateView();
}

	}
}
