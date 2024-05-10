/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Editor/PropertiesView.h"
#include "I18N/Text.h"
#include "Input/Binding/IInputNode.h"
#include "Input/Binding/IInputSourceData.h"
#include "Input/Binding/InputMappingSourceData.h"
#include "Input/Binding/InputMappingStateData.h"
#include "Input/Binding/InputStateData.h"
#include "Input/Binding/InBoolean.h"
#include "Input/Binding/InClamp.h"
#include "Input/Binding/InCombine.h"
#include "Input/Binding/InConst.h"
#include "Input/Binding/InDelay.h"
#include "Input/Binding/InEnvelope.h"
#include "Input/Binding/InGesturePinch.h"
#include "Input/Binding/InGestureSwipe.h"
#include "Input/Binding/InGestureTap.h"
#include "Input/Binding/InHysteresis.h"
#include "Input/Binding/InLowPass.h"
#include "Input/Binding/InPulse.h"
#include "Input/Binding/InReadValue.h"
#include "Input/Binding/InRemapAxis.h"
#include "Input/Binding/InThreshold.h"
#include "Input/Binding/InTrigger.h"
#include "Input/Editor/InputMappingAsset.h"
#include "Input/Editor/InputMappingEditor.h"
#include "Input/Editor/InBooleanTraits.h"
#include "Input/Editor/InClampTraits.h"
#include "Input/Editor/InCombineTraits.h"
#include "Input/Editor/InConstTraits.h"
#include "Input/Editor/InDelayTraits.h"
#include "Input/Editor/InEnvelopeTraits.h"
#include "Input/Editor/InGesturePinchTraits.h"
#include "Input/Editor/InGestureSwipeTraits.h"
#include "Input/Editor/InGestureTapTraits.h"
#include "Input/Editor/InHysteresisTraits.h"
#include "Input/Editor/InLowPassTraits.h"
#include "Input/Editor/InPulseTraits.h"
#include "Input/Editor/InReadValueTraits.h"
#include "Input/Editor/InRemapAxisTraits.h"
#include "Input/Editor/InThresholdTraits.h"
#include "Input/Editor/InTriggerTraits.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/EditList.h"
#include "Ui/EditListEditEvent.h"
#include "Ui/InputDialog.h"
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

namespace traktor::input
{
	namespace
	{

void createInputNodes(InputMappingAsset* mappingAsset, ui::GraphControl* graph, const std::map< const TypeInfo*, Ref< const InputNodeTraits > >& traits, const IInputNode* node, ui::Pin* parentInputPin)
{
	if (!node)
		return;

	auto it = traits.find(&type_of(node));
	T_ASSERT(it != traits.end());

	const InputNodeTraits* t = it->second;
	T_ASSERT(t);

	Ref< ui::Node > graphNode;
	Ref< ui::Pin > valuePin;

	// Have we already created this node?
	for (auto nodeIt : graph->getNodes())
	{
		if (nodeIt->getData< IInputNode >(L"DATA") == node)
		{
			graphNode = nodeIt;
			valuePin = nodeIt->findOutputPin(L"Value");
			T_ASSERT(valuePin);
			break;
		}
	}

	// If nop, then create graph node and recurse with it's children.
	if (!graphNode)
	{
		InputMappingAsset::Position p = mappingAsset->getPosition(node);

		std::map< const std::wstring, Ref< const IInputNode > > childNodes;
		t->getInputNodes(node, childNodes);

		Ref< ui::Node > graphNode = graph->createNode(
			t->getHeader(node),
			t->getDescription(node),
			ui::UnitPoint(ui::Unit(p.x), ui::Unit(p.y)),
			childNodes.empty() ? (ui::INodeShape*)new ui::InputNodeShape() : (ui::INodeShape*)new ui::DefaultNodeShape(ui::DefaultNodeShape::StDefault)
		);
		valuePin = graphNode->createOutputPin(L"Value", Guid());
		graphNode->setData(L"DATA", const_cast< IInputNode* >(node));

		for (auto it : childNodes)
		{
			Ref< ui::Pin > pin = graphNode->createInputPin(it.first, Guid(), false, false);
			createInputNodes(mappingAsset, graph, traits, it.second, pin);
		}
	}

	// Add edge to parent's input pin.
	if (parentInputPin)
	{
		T_ASSERT(valuePin);
		graph->addEdge(new ui::Edge(
			valuePin,
			parentInputPin
		));
	}
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputMappingEditor", InputMappingEditor, editor::IEditorPage)

InputMappingEditor::InputMappingEditor(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
{
	m_traits[&type_of< InBoolean >()] = new InBooleanTraits();
	m_traits[&type_of< InClamp >()] = new InClampTraits();
	m_traits[&type_of< InCombine >()] = new InCombineTraits();
	m_traits[&type_of< InConst >()] = new InConstTraits();
	m_traits[&type_of< InDelay >()] = new InDelayTraits();
	m_traits[&type_of< InEnvelope >()] = new InEnvelopeTraits();
	m_traits[&type_of< InGesturePinch >()] = new InGesturePinchTraits();
	m_traits[&type_of< InGestureSwipe >()] = new InGestureSwipeTraits();
	m_traits[&type_of< InGestureTap >()] = new InGestureTapTraits();
	m_traits[&type_of< InHysteresis >()] = new InHysteresisTraits();
	m_traits[&type_of< InLowPass >()] = new InLowPassTraits();
	m_traits[&type_of< InPulse >()] = new InPulseTraits();
	m_traits[&type_of< InReadValue >()] = new InReadValueTraits();
	m_traits[&type_of< InRemapAxis >()] = new InRemapAxisTraits();
	m_traits[&type_of< InThreshold >()] = new InThresholdTraits();
	m_traits[&type_of< InTrigger >()] = new InTriggerTraits();
}

bool InputMappingEditor::create(ui::Container* parent)
{
	m_mappingAsset = m_document->getObject< InputMappingAsset >(0);
	if (!m_mappingAsset)
		return false;

	if (!m_mappingAsset->getSourceData())
		m_mappingAsset->setSourceData(new InputMappingSourceData());
	if (!m_mappingAsset->getStateData())
		m_mappingAsset->setStateData(new InputMappingStateData());

	InputMappingStateData* stateData = m_mappingAsset->getStateData();
	T_ASSERT(stateData);

	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut));

	m_toolBarGraph = new ui::ToolBar();
	m_toolBarGraph->create(container);
	for (int32_t i = 0; i < 6; ++i)
		m_toolBarGraph->addImage(new ui::StyleBitmap(L"Input.Alignment", i));
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"INPUT_EDITOR_ALIGN_LEFT"), 0, ui::Command(L"Input.Editor.AlignLeft")));
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"INPUT_EDITOR_ALIGN_RIGHT"), 1, ui::Command(L"Input.Editor.AlignRight")));
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"INPUT_EDITOR_ALIGN_TOP"), 2, ui::Command(L"Input.Editor.AlignTop")));
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"INPUT_EDITOR_ALIGN_BOTTOM"), 3, ui::Command(L"Input.Editor.AlignBottom")));
	m_toolBarGraph->addItem(new ui::ToolBarSeparator());
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"INPUT_EDITOR_EVEN_VERTICALLY"), 4, ui::Command(L"Input.Editor.EvenSpaceVertically")));
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"INPUT_EDITOR_EVEN_HORIZONTALLY"), 5, ui::Command(L"Input.Editor.EventSpaceHorizontally")));
	m_toolBarGraph->addEventHandler< ui::ToolBarButtonClickEvent >(this, &InputMappingEditor::eventToolBarGraphClick);

	m_graph = new ui::GraphControl();
	m_graph->create(container);
	m_graph->setText(L"INPUT");
	m_graph->addEventHandler< ui::MouseButtonDownEvent >(this, &InputMappingEditor::eventButtonDown);
	m_graph->addEventHandler< ui::SelectEvent >(this, &InputMappingEditor::eventNodeSelect);
	m_graph->addEventHandler< ui::NodeMovedEvent >(this, &InputMappingEditor::eventNodeMoved);
	m_graph->addEventHandler< ui::NodeActivateEvent >(this, &InputMappingEditor::eventNodeActivated);
	m_graph->addEventHandler< ui::EdgeConnectEvent >(this, &InputMappingEditor::eventEdgeConnected);
	m_graph->addEventHandler< ui::EdgeDisconnectEvent >(this, &InputMappingEditor::eventEdgeDisconnected);

	// Create properties view.
	m_propertiesView = m_site->createPropertiesView(parent);
	m_propertiesView->addEventHandler< ui::ContentChangingEvent >(this, &InputMappingEditor::eventPropertiesChanging);
	m_propertiesView->addEventHandler< ui::ContentChangeEvent >(this, &InputMappingEditor::eventPropertiesChanged);
	m_site->createAdditionalPanel(m_propertiesView, 400_ut, false);

	// Create "sources" view.
	m_listValueSources = new ui::EditList();
	m_listValueSources->create(parent, ui::ListBox::WsSingle | ui::EditList::WsAutoAdd | ui::EditList::WsAutoRemove);
	m_listValueSources->setText(i18n::Text(L"INPUT_EDITOR_SOURCES"));
	m_listValueSources->addEventHandler< ui::SelectionChangeEvent >(this, &InputMappingEditor::eventListValueSourceSelect);
	m_listValueSources->addEventHandler< ui::EditListEditEvent >(this, &InputMappingEditor::eventListValueEdit);
	m_site->createAdditionalPanel(m_listValueSources, 400_ut, false);

	InputMappingSourceData* sourceData = m_mappingAsset->getSourceData();
	if (sourceData)
	{
		for (const auto& it : sourceData->getSourceData())
			m_listValueSources->add(it.first, it.second);
	}

	// Build pop-up menu.
	m_menuPopup = new ui::Menu();

	Ref< ui::MenuItem > menuItemCreate = new ui::MenuItem(i18n::Text(L"INPUT_EDITOR_CREATE_NODE"));

	Ref< ui::MenuItem > menuItem = new ui::MenuItem(ui::Command(L"Input.Editor.Create"), i18n::Text(L"INPUT_EDITOR_CREATE_NODE_STATE"));
	menuItemCreate->add(menuItem);

	for (auto it : m_traits)
	{
		std::wstring i18nId = it.first->getName();
		i18nId = replaceAll(i18nId, L'.', L'_');
		i18nId = toUpper(i18nId);

		Ref< ui::MenuItem > menuItem = new ui::MenuItem(ui::Command(L"Input.Editor.Create"), i18n::Text(L"INPUT_EDITOR_CREATE_NODE_" + i18nId));
		menuItem->setData(L"TRAITS", const_cast< InputNodeTraits* >(it.second.ptr()));
		menuItemCreate->add(menuItem);
	}

	m_menuPopup->add(menuItemCreate);
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"INPUT_EDITOR_DELETE_NODE")));

	updateGraphView();
	return true;
}

void InputMappingEditor::destroy()
{
	if (m_propertiesView)
		m_site->destroyAdditionalPanel(m_propertiesView);
	if (m_listValueSources)
		m_site->destroyAdditionalPanel(m_listValueSources);

	safeDestroy(m_propertiesView);
	safeDestroy(m_listValueSources);

	m_site = nullptr;
}

bool InputMappingEditor::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool InputMappingEditor::handleCommand(const ui::Command& command)
{
	if (m_propertiesView->handleCommand(command))
		return true;

	if (command == L"Editor.Undo")
	{
		if (m_document->undo())
		{
			m_mappingAsset = m_document->getObject< InputMappingAsset >(0);
			T_FATAL_ASSERT (m_mappingAsset);

			updateGraphView();

			m_propertiesView->setPropertyObject(nullptr);
		}
	}
	else if (command == L"Editor.Redo")
	{
		if (m_document->redo())
		{
			m_mappingAsset = m_document->getObject< InputMappingAsset >(0);
			T_FATAL_ASSERT (m_mappingAsset);

			updateGraphView();

			m_propertiesView->setPropertyObject(nullptr);
		}
	}
	else if (command == L"Input.Editor.AlignLeft")
	{
		m_document->push();
		m_graph->alignNodes(ui::GraphControl::AnLeft);
		m_graph->update();
	}
	else if (command == L"Input.Editor.AlignRight")
	{
		m_document->push();
		m_graph->alignNodes(ui::GraphControl::AnRight);
		m_graph->update();
	}
	else if (command == L"Input.Editor.AlignTop")
	{
		m_document->push();
		m_graph->alignNodes(ui::GraphControl::AnTop);
		m_graph->update();
	}
	else if (command == L"Input.Editor.AlignBottom")
	{
		m_document->push();
		m_graph->alignNodes(ui::GraphControl::AnBottom);
		m_graph->update();
	}
	else if (command == L"Input.Editor.EvenSpaceVertically")
	{
		m_document->push();
		m_graph->evenSpace(ui::GraphControl::EsVertically);
		m_graph->update();
	}
	else if (command == L"Input.Editor.EventSpaceHorizontally")
	{
		m_document->push();
		m_graph->evenSpace(ui::GraphControl::EsHorizontally);
		m_graph->update();
	}
	else
		return false;

	return true;
}

void InputMappingEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void InputMappingEditor::updateGraphView()
{
	m_graph->removeAllEdges();
	m_graph->removeAllNodes();

	// Add all input nodes first.
	for (auto inputNode : m_mappingAsset->getInputNodes())
		createInputNodes(m_mappingAsset, m_graph, m_traits, inputNode, nullptr);

	// Add all output states.
	InputMappingStateData* stateData = m_mappingAsset->getStateData();
	T_ASSERT(stateData);

	for (const auto& it : stateData->getStateData())
	{
		InputMappingAsset::Position p = m_mappingAsset->getPosition(it.second);

		Ref< ui::Node > node = m_graph->createNode(
			L"State",
			it.first,
			ui::UnitPoint(ui::Unit(p.x), ui::Unit(p.y)),
			new ui::OutputNodeShape()
		);
		node->setData(L"NAME", new PropertyString(it.first));
		node->setData(L"DATA", it.second);
		Ref< ui::Pin > inputPin = node->createInputPin(L"Input", Guid(), false, false);

		// Create edge to input node.
		if (it.second->getSource() != nullptr)
		{
			for (auto node : m_graph->getNodes())
			{
				if (node->getData< IInputNode >(L"DATA") == it.second->getSource())
				{
					m_graph->addEdge(new ui::Edge(
						node->findOutputPin(L"Value"),
						inputPin
					));
					break;
				}
			}
		}
	}

	m_graph->update();
}

void InputMappingEditor::eventToolBarGraphClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	handleCommand(command);
}

void InputMappingEditor::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtRight)
		return;

	const ui::MenuItem* selected = m_menuPopup->showModal(m_graph, event->getPosition());
	if (!selected)
		return;

	if (selected->getCommand() == L"Input.Editor.Create")
	{
		m_document->push();

		const InputNodeTraits* traits = selected->getData< InputNodeTraits >(L"TRAITS");
		if (traits)
		{
			Ref< IInputNode > node = traits->createNode();
			if (!node)
				return;

			const ui::Point pos = (event->getPosition() - m_graph->getOffset());
			const InputMappingAsset::Position position = { pos.x, pos.y };

			m_mappingAsset->addInputNode(node);
			m_mappingAsset->setPosition(node, position);
		}
		else
		{
			InputMappingStateData* stateData = m_mappingAsset->getStateData();
			T_ASSERT(stateData);

			Ref< InputStateData > sd = new InputStateData();

			const ui::Point pos = (event->getPosition() - m_graph->getOffset());
			const InputMappingAsset::Position position = { pos.x, pos.y };

			m_mappingAsset->setPosition(sd, position);
			stateData->setStateData(L"STATE_UNNAMED_" + toString(uint32_t(stateData->getStateData().size())), sd);
		}

		updateGraphView();
	}
	else if (selected->getCommand() == L"Editor.Delete")
	{
		m_document->push();
		for (auto selectedNode : m_graph->getSelectedNodes())
		{
			for (auto outputPin : selectedNode->getOutputPins())
			{
				for (auto edge : m_graph->getConnectedEdges(outputPin))
				{
					ui::Pin* destinationPin = edge->getDestinationPin();
					ui::Node* destinationNode = destinationPin->getNode();

					IInputNode* destinationInputNode = destinationNode->getData< IInputNode >(L"DATA");
					if (destinationInputNode)
					{
						const auto it = m_traits.find(&type_of(destinationInputNode));
						T_ASSERT(it != m_traits.end());

						const InputNodeTraits* destinationTraits = it->second;
						T_ASSERT(destinationTraits);

						destinationTraits->disconnectInputNode(destinationInputNode, destinationPin->getName());
					}

					InputStateData* destinationStateData = destinationNode->getData< InputStateData >(L"DATA");
					if (destinationStateData)
					{
						destinationStateData->setSource(0);
					}
				}
			}

			Ref< IInputNode > sourceInputNode = selectedNode->getData< IInputNode >(L"DATA");
			if (sourceInputNode)
				m_mappingAsset->removeInputNode(sourceInputNode);

			Ref< PropertyString > stateName = selectedNode->getData< PropertyString >(L"NAME");
			Ref< InputStateData > stateData = selectedNode->getData< InputStateData >(L"DATA");
			if (stateName && stateData)
			{
				const std::wstring currentName = PropertyString::get(stateName);
				m_mappingAsset->getStateData()->setStateData(currentName, nullptr);
			}
		}

		updateGraphView();
	}
}

void InputMappingEditor::eventListValueSourceSelect(ui::SelectionChangeEvent* event)
{
	Ref< IInputSourceData > sourceData = m_listValueSources->getSelectedData< IInputSourceData >();
	if (sourceData)
		m_propertiesView->setPropertyObject(sourceData);
}

void InputMappingEditor::eventNodeSelect(ui::SelectEvent* event)
{
	const RefArray< ui::Node >& nodes = event->getNodes();
	if (nodes.size() == 1)
		m_propertiesView->setPropertyObject(nodes[0]->getData< ISerializable >(L"DATA"));
}

void InputMappingEditor::eventListValueEdit(ui::EditListEditEvent* event)
{
	InputMappingSourceData* sourceData = m_mappingAsset->getSourceData();
	T_ASSERT(sourceData);

	if (event->getIndex() < 0)	// Add item.
	{
		// Ensure name doesn't clash with existing.
		const auto& sd = sourceData->getSourceData();
		if (sd.find(event->getText()) != sd.end())
			return;

		// Browse for type of source.
		const TypeInfo* inputSourceDataType = m_editor->browseType(makeTypeInfoSet< IInputSourceData >(), false, true);
		if (!inputSourceDataType)
			return;

		// Add source to mapping.
		Ref< IInputSourceData > inputSourceData = checked_type_cast< IInputSourceData*, false >(inputSourceDataType->createInstance());
		sourceData->setSourceData(event->getText(), inputSourceData);

		// Add new item to list.
		m_listValueSources->add(event->getText(), inputSourceData);
	}
	else if (event->getText().empty())	// Remove item.
	{
		const std::wstring id = m_listValueSources->getItem(event->getIndex());
		sourceData->setSourceData(id, nullptr);
		event->consume();
	}
	else	// Rename item.
	{
		// Ensure name doesn't clash with existing.
		const auto& sd = sourceData->getSourceData();
		if (sd.find(event->getText()) != sd.end())
			return;

		const std::wstring fromId = m_listValueSources->getItem(event->getIndex());
		Ref< IInputSourceData > inputSourceData = sourceData->getSourceData(fromId);
		sourceData->setSourceData(fromId, nullptr);
		sourceData->setSourceData(event->getText(), inputSourceData);
		event->consume();
	}
}

void InputMappingEditor::eventNodeMoved(ui::NodeMovedEvent* event)
{
	ui::Node* node = event->getNode();
	T_ASSERT(node);

	const ui::UnitPoint position = node->getPosition();
	const InputMappingAsset::Position p =
	{
		position.x.get(),
		position.y.get()
	};

	m_mappingAsset->setPosition(node->getData(L"DATA"), p);
}

void InputMappingEditor::eventNodeActivated(ui::NodeActivateEvent* event)
{
	ui::Node* node = event->getNode();
	T_ASSERT(node);

	Ref< PropertyString > stateName = node->getData< PropertyString >(L"NAME");
	Ref< InputStateData > stateData = node->getData< InputStateData >(L"DATA");
	if (stateName && stateData)
	{
		std::wstring currentName = PropertyString::get(stateName);

		ui::InputDialog::Field fields[] =
		{
			ui::InputDialog::Field(i18n::Text(L"INPUT_EDITOR_STATE_NAME"), currentName)
		};

		ui::InputDialog inputDialog;
		inputDialog.create(
			m_graph,
			i18n::Text(L"INPUT_EDITOR_STATE_NAME_TITLE"),
			i18n::Text(L"INPUT_EDITOR_STATE_NAME_MESSAGE"),
			fields,
			sizeof_array(fields)
		);
		if (inputDialog.showModal() == ui::DialogResult::Ok && !fields[0].value.empty())
		{
			InputMappingStateData* mappingStateData = m_mappingAsset->getStateData();
			T_ASSERT(mappingStateData);

			mappingStateData->setStateData(currentName, 0);
			mappingStateData->setStateData(fields[0].value, stateData);

			updateGraphView();
		}
		inputDialog.destroy();
	}
}

void InputMappingEditor::eventEdgeConnected(ui::EdgeConnectEvent* event)
{
	ui::Edge* edge = event->getEdge();
	ui::Pin* sourcePin = edge->getSourcePin();
	ui::Pin* destinationPin = edge->getDestinationPin();
	ui::Node* sourceNode = sourcePin->getNode();
	ui::Node* destinationNode = destinationPin->getNode();

	IInputNode* sourceInputNode = sourceNode->getData< IInputNode >(L"DATA");
	T_ASSERT(sourceInputNode);

	IInputNode* destinationInputNode = destinationNode->getData< IInputNode >(L"DATA");
	if (destinationInputNode)
	{
		const auto it = m_traits.find(&type_of(destinationInputNode));
		T_ASSERT(it != m_traits.end());

		const InputNodeTraits* destinationTraits = it->second;
		T_ASSERT(destinationTraits);

		destinationTraits->connectInputNode(destinationInputNode, destinationPin->getName(), sourceInputNode);
	}

	InputStateData* destinationStateData = destinationNode->getData< InputStateData >(L"DATA");
	if (destinationStateData)
	{
		destinationStateData->setSource(sourceInputNode);
	}

	updateGraphView();
}

void InputMappingEditor::eventEdgeDisconnected(ui::EdgeDisconnectEvent* event)
{
	ui::Edge* edge = event->getEdge();
	ui::Pin* destinationPin = edge->getDestinationPin();
	ui::Node* destinationNode = destinationPin->getNode();

	IInputNode* destinationInputNode = destinationNode->getData< IInputNode >(L"DATA");
	if (destinationInputNode)
	{
		const auto it = m_traits.find(&type_of(destinationInputNode));
		T_ASSERT(it != m_traits.end());

		const InputNodeTraits* destinationTraits = it->second;
		T_ASSERT(destinationTraits);

		destinationTraits->disconnectInputNode(destinationInputNode, destinationPin->getName());
	}

	InputStateData* destinationStateData = destinationNode->getData< InputStateData >(L"DATA");
	if (destinationStateData)
	{
		destinationStateData->setSource(nullptr);
	}

	updateGraphView();
}

void InputMappingEditor::eventPropertiesChanging(ui::ContentChangingEvent* event)
{
	m_document->push();
}

void InputMappingEditor::eventPropertiesChanged(ui::ContentChangeEvent* event)
{
	updateGraphView();
}

}
