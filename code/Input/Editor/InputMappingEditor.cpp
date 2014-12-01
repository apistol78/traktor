#include "Core/Misc/String.h"

#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Input/Binding/IInputNode.h"
#include "Input/Binding/IInputSourceData.h"
#include "Input/Binding/InputMappingSourceData.h"
#include "Input/Binding/InputMappingStateData.h"
#include "Input/Binding/InputStateData.h"

#include "Input/Binding/InBoolean.h"
#include "Input/Binding/InClamp.h"
#include "Input/Binding/InCombine.h"
#include "Input/Binding/InConst.h"
#include "Input/Binding/InGesturePinch.h"
#include "Input/Binding/InGestureSwipe.h"
#include "Input/Binding/InGestureTap.h"
#include "Input/Binding/InHysteresis.h"
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
#include "Input/Editor/InGesturePinchTraits.h"
#include "Input/Editor/InGestureSwipeTraits.h"
#include "Input/Editor/InGestureTapTraits.h"
#include "Input/Editor/InHysteresisTraits.h"
#include "Input/Editor/InPulseTraits.h"
#include "Input/Editor/InReadValueTraits.h"
#include "Input/Editor/InRemapAxisTraits.h"
#include "Input/Editor/InThresholdTraits.h"
#include "Input/Editor/InTriggerTraits.h"

#include "Ui/Container.h"
#include "Ui/MenuItem.h"
#include "Ui/PopupMenu.h"
#include "Ui/TableLayout.h"

#include "Ui/Custom/EditList.h"
#include "Ui/Custom/EditListEditEvent.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/Graph/DefaultNodeShape.h"
#include "Ui/Custom/Graph/Edge.h"
#include "Ui/Custom/Graph/EdgeConnectEvent.h"
#include "Ui/Custom/Graph/EdgeDisconnectEvent.h"
#include "Ui/Custom/Graph/GraphControl.h"
#include "Ui/Custom/Graph/InputNodeShape.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/NodeMovedEvent.h"
#include "Ui/Custom/Graph/OutputNodeShape.h"
#include "Ui/Custom/Graph/Pin.h"
#include "Ui/Custom/Graph/SelectEvent.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

// This is used to ensure mapping asset have it's input node instances
// in a set which is kept even if no references to the input node exists.
void updateInputMappingAsset(InputMappingAsset* mappingAsset, const IInputNode* node, const std::map< const TypeInfo*, Ref< const InputNodeTraits > >& traits)
{
	if (!node)
		return;

	// Use traits to find child input nodes.
	std::map< const TypeInfo*, Ref< const InputNodeTraits > >::const_iterator it = traits.find(&type_of(node));
	T_ASSERT (it != traits.end());

	const InputNodeTraits* t = it->second;
	T_ASSERT (t);

	std::map< const std::wstring, Ref< const IInputNode > > childNodes;
	t->getInputNodes(node, childNodes);

	for (std::map< const std::wstring, Ref< const IInputNode > >::const_iterator i = childNodes.begin(); i != childNodes.end(); ++i)
		updateInputMappingAsset(mappingAsset, i->second, traits);

	// Add input node to asset.
	mappingAsset->addInputNode(const_cast< IInputNode* >(node));
}

void createInputNodes(InputMappingAsset* mappingAsset, ui::custom::GraphControl* graph, const std::map< const TypeInfo*, Ref< const InputNodeTraits > >& traits, const IInputNode* node, ui::custom::Pin* parentInputPin)
{
	if (!node)
		return;

	std::map< const TypeInfo*, Ref< const InputNodeTraits > >::const_iterator it = traits.find(&type_of(node));
	T_ASSERT (it != traits.end());

	const InputNodeTraits* t = it->second;
	T_ASSERT (t);

	Ref< ui::custom::Node > graphNode;
	Ref< ui::custom::Pin > valuePin;

	// Have we already created this node?
	const RefArray< ui::custom::Node >& nodes = graph->getNodes();
	for (RefArray< ui::custom::Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		if ((*i)->getData< IInputNode >(L"DATA") == node)
		{
			graphNode = *i;
			valuePin = (*i)->findOutputPin(L"Value");
			T_ASSERT (valuePin);
			break;
		}
	}

	// If nop, then create graph node and recurse with it's children.
	if (!graphNode)
	{
		InputMappingAsset::Position p = mappingAsset->getPosition(node);

		std::map< const std::wstring, Ref< const IInputNode > > childNodes;
		t->getInputNodes(node, childNodes);

		Ref< ui::custom::Node > graphNode = new ui::custom::Node(
			t->getHeader(node),
			t->getDescription(node),
			ui::Point(p.x, p.y),
			childNodes.empty() ? ((ui::custom::NodeShape*)new ui::custom::InputNodeShape(graph)) : ((ui::custom::NodeShape*)new ui::custom::DefaultNodeShape(graph))
		);
		valuePin = graphNode->createOutputPin(L"Value");
		graphNode->setData(L"DATA", const_cast< IInputNode* >(node));

		for (std::map< const std::wstring, Ref< const IInputNode > >::const_iterator i = childNodes.begin(); i != childNodes.end(); ++i)
		{
			Ref< ui::custom::Pin > pin = graphNode->createInputPin(i->first, false);
			createInputNodes(mappingAsset, graph, traits, i->second, pin);
		}

		graph->addNode(graphNode);
	}

	// Add edge to parent's input pin.
	if (parentInputPin)
	{
		T_ASSERT (valuePin);
		graph->addEdge(new ui::custom::Edge(
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
	m_traits[&type_of< InGesturePinch >()] = new InGesturePinchTraits();
	m_traits[&type_of< InGestureSwipe >()] = new InGestureSwipeTraits();
	m_traits[&type_of< InGestureTap >()] = new InGestureTapTraits();
	m_traits[&type_of< InHysteresis >()] = new InHysteresisTraits();
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

	InputMappingStateData* stateData = m_mappingAsset->getStateData();
	if (stateData)
	{
		const std::map< std::wstring, Ref< InputStateData > >& sd = stateData->getStateData();
		for (std::map< std::wstring, Ref< InputStateData > >::const_iterator i = sd.begin(); i != sd.end(); ++i)
			updateInputMappingAsset(m_mappingAsset, i->second->getSource(), m_traits);
	}

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(parent, true, 10, true);

	m_listValueSources = new ui::custom::EditList();
	m_listValueSources->create(splitter, ui::ListBox::WsSingle | ui::custom::EditList::WsAutoAdd | ui::custom::EditList::WsAutoRemove);
	m_listValueSources->addEventHandler< ui::SelectionChangeEvent >(this, &InputMappingEditor::eventListValueSourceSelect);
	m_listValueSources->addEventHandler< ui::custom::EditListEditEvent >(this, &InputMappingEditor::eventListValueEdit);

	m_graph = new ui::custom::GraphControl();
	m_graph->create(splitter);
	m_graph->addEventHandler< ui::MouseButtonDownEvent >(this, &InputMappingEditor::eventButtonDown);
	m_graph->addEventHandler< ui::custom::SelectEvent >(this, &InputMappingEditor::eventNodeSelect);
	m_graph->addEventHandler< ui::custom::NodeMovedEvent >(this, &InputMappingEditor::eventNodeMoved);
	m_graph->addEventHandler< ui::custom::EdgeConnectEvent >(this, &InputMappingEditor::eventEdgeConnected);
	m_graph->addEventHandler< ui::custom::EdgeDisconnectEvent >(this, &InputMappingEditor::eventEdgeDisconnected);

	InputMappingSourceData* sourceData = m_mappingAsset->getSourceData();
	if (sourceData)
	{
		const std::map< std::wstring, Ref< IInputSourceData > >& sd = sourceData->getSourceData();
		for (std::map< std::wstring, Ref< IInputSourceData > >::const_iterator i = sd.begin(); i != sd.end(); ++i)
			m_listValueSources->add(i->first, i->second);
	}

	// Build popup menu.
	m_menuPopup = new ui::PopupMenu();
	m_menuPopup->create();

	Ref< ui::MenuItem > menuItemCreate = new ui::MenuItem(L"Create"); // i18n::Text(L"INPUT_EDITOR_CREATE_NODE"));

	Ref< ui::MenuItem > menuItem = new ui::MenuItem(ui::Command(L"Input.Editor.Create"), L"State");
	menuItemCreate->add(menuItem);

	for (std::map< const TypeInfo*, Ref< const InputNodeTraits > >::const_iterator i = m_traits.begin(); i != m_traits.end(); ++i)
	{
		Ref< ui::MenuItem > menuItem = new ui::MenuItem(ui::Command(L"Input.Editor.Create"), i->first->getName());
		menuItem->setData(L"TRAITS", const_cast< InputNodeTraits* >(i->second.ptr()));
		menuItemCreate->add(menuItem);
	}

	m_menuPopup->add(menuItemCreate);
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), L"Delete")); //, i18n::Text(L"SHADERGRAPH_DELETE_NODE")));

	updateGraphView();
	return true;
}

void InputMappingEditor::destroy()
{
}

void InputMappingEditor::activate()
{
}

void InputMappingEditor::deactivate()
{
}

bool InputMappingEditor::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool InputMappingEditor::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.PropertiesChanged")
	{
		updateGraphView();
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
	const RefSet< IInputNode >& inputNodes = m_mappingAsset->getInputNodes();
	for (RefSet< IInputNode >::const_iterator i = inputNodes.begin(); i != inputNodes.end(); ++i)
		createInputNodes(m_mappingAsset, m_graph, m_traits, *i, 0);

	// Add all output states.
	InputMappingStateData* stateData = m_mappingAsset->getStateData();
	if (stateData)
	{
		const std::map< std::wstring, Ref< InputStateData > >& sd = stateData->getStateData();
		for (std::map< std::wstring, Ref< InputStateData > >::const_iterator i = sd.begin(); i != sd.end(); ++i)
		{
			InputMappingAsset::Position p = m_mappingAsset->getPosition(i->second);

			Ref< ui::custom::Node > node = new ui::custom::Node(
				L"State",
				i->first,
				ui::Point(p.x, p.y),
				new ui::custom::OutputNodeShape(m_graph)
			);
			node->setData(L"DATA", i->second);
			Ref< ui::custom::Pin > inputPin = node->createInputPin(L"Input", false);
			m_graph->addNode(node);

			// Create edge to input node.
			if (i->second->getSource() != 0)
			{
				const RefArray< ui::custom::Node >& nodes = m_graph->getNodes();
				for (RefArray< ui::custom::Node >::const_iterator j = nodes.begin(); j != nodes.end(); ++j)
				{
					if ((*j)->getData< IInputNode >(L"DATA") == i->second->getSource())
					{
						m_graph->addEdge(new ui::custom::Edge(
							(*j)->findOutputPin(L"Value"),
							inputPin
						));
						break;
					}
				}
			}
		}
	}

	m_graph->update();
}

void InputMappingEditor::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtRight)
		return;

	Ref< ui::MenuItem > selected = m_menuPopup->show(m_graph, event->getPosition());
	if (!selected)
		return;

	if (selected->getCommand() == L"Input.Editor.Create")
	{
		const InputNodeTraits* traits = selected->getData< InputNodeTraits >(L"TRAITS");
		if (traits)
		{
			Ref< IInputNode > node = traits->createNode();
			if (!node)
				return;

			InputMappingAsset::Position position =
			{
				event->getPosition().x,
				event->getPosition().y
			};

			m_mappingAsset->addInputNode(node);
			m_mappingAsset->setPosition(node, position);
		}
		else
		{
			InputMappingStateData* stateData = m_mappingAsset->getStateData();
			if (stateData)
			{
				Ref< InputStateData > sd = new InputStateData();

				InputMappingAsset::Position position =
				{
					event->getPosition().x,
					event->getPosition().y
				};

				m_mappingAsset->setPosition(sd, position);
				stateData->setStateData(L"STATE_UNNAMED_" + toString(stateData->getStateData().size()), sd);
			}
		}

		updateGraphView();
	}
	else if (selected->getCommand() == L"Editor.Delete")
	{
		RefArray< ui::custom::Node > selectedNodes;
		m_graph->getSelectedNodes(selectedNodes);

		for (RefArray< ui::custom::Node >::const_iterator i = selectedNodes.begin(); i != selectedNodes.end(); ++i)
		{
			const RefArray< ui::custom::Pin >& outputPins = (*i)->getOutputPins();
			for (RefArray< ui::custom::Pin >::const_iterator j = outputPins.begin(); j != outputPins.end(); ++j)
			{
				RefArray< ui::custom::Edge > edges;
				m_graph->getConnectedEdges(*j, edges);

				for (RefArray< ui::custom::Edge >::const_iterator k = edges.begin(); k != edges.end(); ++k)
				{
					ui::custom::Edge* edge = *k;
					ui::custom::Pin* destinationPin = edge->getDestinationPin();
					ui::custom::Node* destinationNode = destinationPin->getNode();

					IInputNode* destinationInputNode = destinationNode->getData< IInputNode >(L"DATA");
					if (destinationInputNode)
					{
						std::map< const TypeInfo*, Ref< const InputNodeTraits > >::const_iterator it = m_traits.find(&type_of(destinationInputNode));
						T_ASSERT (it != m_traits.end());

						const InputNodeTraits* destinationTraits = it->second;
						T_ASSERT (destinationTraits);

						destinationTraits->disconnectInputNode(destinationInputNode, destinationPin->getName());
					}

					InputStateData* destinationStateData = destinationNode->getData< InputStateData >(L"DATA");
					if (destinationStateData)
					{
						destinationStateData->setSource(0);
					}
				}
			}

			IInputNode* sourceInputNode = (*i)->getData< IInputNode >(L"DATA");
			if (sourceInputNode)
				m_mappingAsset->removeInputNode(sourceInputNode);

			// \fixme Else remove state node.
		}

		updateGraphView();
	}
}

void InputMappingEditor::eventListValueSourceSelect(ui::SelectionChangeEvent* event)
{
	Ref< IInputSourceData > sourceData = m_listValueSources->getSelectedData< IInputSourceData >();
	if (sourceData)
		m_site->setPropertyObject(sourceData);
}

void InputMappingEditor::eventNodeSelect(ui::custom::SelectEvent* event)
{
	const RefArray< ui::custom::Node >& nodes = event->getNodes();
	if (nodes.size() == 1)
		m_site->setPropertyObject(nodes[0]->getData(L"DATA"));
}

void InputMappingEditor::eventListValueEdit(ui::custom::EditListEditEvent* event)
{
}

void InputMappingEditor::eventNodeMoved(ui::custom::NodeMovedEvent* event)
{
	ui::custom::Node* node = event->getNode();
	T_ASSERT (node);

	InputMappingAsset::Position p =
	{
		node->getPosition().x,
		node->getPosition().y
	};

	m_mappingAsset->setPosition(node->getData(L"DATA"), p);
}

void InputMappingEditor::eventEdgeConnected(ui::custom::EdgeConnectEvent* event)
{
	ui::custom::Edge* edge = event->getEdge();
	ui::custom::Pin* sourcePin = edge->getSourcePin();
	ui::custom::Pin* destinationPin = edge->getDestinationPin();
	ui::custom::Node* sourceNode = sourcePin->getNode();
	ui::custom::Node* destinationNode = destinationPin->getNode();

	IInputNode* sourceInputNode = sourceNode->getData< IInputNode >(L"DATA");
	T_ASSERT (sourceInputNode);

	IInputNode* destinationInputNode = destinationNode->getData< IInputNode >(L"DATA");
	if (destinationInputNode)
	{
		std::map< const TypeInfo*, Ref< const InputNodeTraits > >::const_iterator it = m_traits.find(&type_of(destinationInputNode));
		T_ASSERT (it != m_traits.end());

		const InputNodeTraits* destinationTraits = it->second;
		T_ASSERT (destinationTraits);

		destinationTraits->connectInputNode(destinationInputNode, destinationPin->getName(), sourceInputNode);
	}

	InputStateData* destinationStateData = destinationNode->getData< InputStateData >(L"DATA");
	if (destinationStateData)
	{
		destinationStateData->setSource(sourceInputNode);
	}

	updateGraphView();
}

void InputMappingEditor::eventEdgeDisconnected(ui::custom::EdgeDisconnectEvent* event)
{
	ui::custom::Edge* edge = event->getEdge();
	ui::custom::Pin* destinationPin = edge->getDestinationPin();
	ui::custom::Node* destinationNode = destinationPin->getNode();

	IInputNode* destinationInputNode = destinationNode->getData< IInputNode >(L"DATA");
	if (destinationInputNode)
	{
		std::map< const TypeInfo*, Ref< const InputNodeTraits > >::const_iterator it = m_traits.find(&type_of(destinationInputNode));
		T_ASSERT (it != m_traits.end());

		const InputNodeTraits* destinationTraits = it->second;
		T_ASSERT (destinationTraits);

		destinationTraits->disconnectInputNode(destinationInputNode, destinationPin->getName());
	}

	InputStateData* destinationStateData = destinationNode->getData< InputStateData >(L"DATA");
	if (destinationStateData)
	{
		destinationStateData->setSource(0);
	}

	updateGraphView();
}

	}
}
