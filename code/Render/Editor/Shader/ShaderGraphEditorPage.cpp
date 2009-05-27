#include "Render/Editor/Shader/ShaderGraphEditorPage.h"
#include "Render/Editor/Shader/ShaderGraphEditorClipboardData.h"
#include "Render/Editor/Shader/NodeFacade.h"
#include "Render/Editor/Shader/DefaultNodeFacade.h"
#include "Render/Editor/Shader/InterpolatorNodeFacade.h"
#include "Render/Editor/Shader/SwitchNodeFacade.h"
#include "Render/Editor/Shader/SwizzleNodeFacade.h"
#include "Render/Editor/Shader/ExternalNodeFacade.h"
#include "Render/Editor/Shader/SamplerNodeFacade.h"
#include "Render/Editor/Shader/NodeCategories.h"
#include "Render/Editor/Shader/ShaderGraphValidator.h"
#include "Render/Editor/TextureAsset.h"
#include "Render/ShaderGraph.h"
#include "Render/ShaderGraphAdjacency.h"
#include "Render/Edge.h"
#include "Editor/Editor.h"
#include "Editor/Settings.h"
#include "Editor/BrowseFilter.h"
#include "Editor/UndoStack.h"
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
#include "I18N/Text.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Core/Misc/StringUtils.h"
#include "Core/Log/Log.h"

// Resources
#include "Resources/Alignment.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class RefereeFilter : public editor::BrowseFilter
{
public:
	RefereeFilter(const Guid& fragmentGuid)
	:	m_fragmentGuid(fragmentGuid)
	{
	}

	virtual bool acceptable(db::Instance* instance) const
	{
		Ref< ShaderGraph > shaderGraph = instance->checkout< ShaderGraph >(db::CfReadOnly);
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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphEditorPage", ShaderGraphEditorPage, editor::EditorPage)

ShaderGraphEditorPage::ShaderGraphEditorPage(editor::Editor* editor)
:	m_editor(editor)
,	m_lastValidationResult(true)
{
}

bool ShaderGraphEditorPage::create(ui::Container* parent)
{
	Ref< ui::Container > container = gc_new< ui::Container >();
	container->create(parent, ui::WsNone, gc_new< ui::TableLayout >(L"100%", L"*,100%", 0, 0));

	// Create our custom toolbar.
	m_toolBar = gc_new< ui::custom::ToolBar >();
	m_toolBar->create(container);
	m_toolBar->addImage(ui::Bitmap::load(c_ResourceAlignment, sizeof(c_ResourceAlignment), L"png"), 8);
	m_toolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SHADERGRAPH_OPEN_REFEREE"), ui::Command(L"ShaderGraph.Editor.OpenReferee"), 6));
	m_toolBar->addItem(gc_new< ui::custom::ToolBarSeparator >());
	m_toolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SHADERGRAPH_ALIGN_LEFT"), ui::Command(L"ShaderGraph.Editor.AlignLeft"), 0));
	m_toolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SHADERGRAPH_ALIGN_RIGHT"), ui::Command(L"ShaderGraph.Editor.AlignRight"), 1));
	m_toolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SHADERGRAPH_ALIGN_TOP"), ui::Command(L"ShaderGraph.Editor.AlignTop"), 2));
	m_toolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SHADERGRAPH_ALIGN_BOTTOM"), ui::Command(L"ShaderGraph.Editor.AlignBottom"), 3));
	m_toolBar->addItem(gc_new< ui::custom::ToolBarSeparator >());
	m_toolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SHADERGRAPH_EVEN_VERTICALLY"), ui::Command(L"ShaderGraph.Editor.EvenSpaceVertically"), 4));
	m_toolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SHADERGRAPH_EVEN_HORIZONTALLY"), ui::Command(L"ShaderGraph.Editor.EventSpaceHorizontally"), 5));
	m_toolBar->addItem(gc_new< ui::custom::ToolBarSeparator >());
	m_toolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SHADERGRAPH_CENTER"), ui::Command(L"ShaderGraph.Editor.Center"), 7));
	m_toolBar->addClickEventHandler(ui::createMethodHandler(this, &ShaderGraphEditorPage::eventToolClick));

	// Create shader graph editor control.
	m_editorGraph = gc_new< ui::custom::GraphControl >();
	m_editorGraph->create(container);
	m_editorGraph->addButtonDownEventHandler(ui::createMethodHandler(this, &ShaderGraphEditorPage::eventButtonDown));
	m_editorGraph->addSelectEventHandler(ui::createMethodHandler(this, &ShaderGraphEditorPage::eventSelect));
	m_editorGraph->addNodeMovedEventHandler(ui::createMethodHandler(this, &ShaderGraphEditorPage::eventNodeMoved));
	m_editorGraph->addNodeDoubleClickEventHandler(ui::createMethodHandler(this, &ShaderGraphEditorPage::eventNodeDoubleClick));
	m_editorGraph->addEdgeConnectEventHandler(ui::createMethodHandler(this, &ShaderGraphEditorPage::eventEdgeConnect));
	m_editorGraph->addEdgeDisconnectEventHandler(ui::createMethodHandler(this, &ShaderGraphEditorPage::eventEdgeDisconnect));

	// Modify graph control settings.
	Ref< ui::custom::PaintSettings > paintSettings = m_editorGraph->getPaintSettings();
	paintSettings->setSmoothSpline(m_editor->getSettings()->getProperty< editor::PropertyBoolean >(L"ShaderEditor.SmoothSpline"));

	// Build popup menu.
	m_menuPopup = gc_new< ui::PopupMenu >();
	m_menuPopup->create();
	Ref< ui::MenuItem > menuItemCreate = gc_new< ui::MenuItem >(i18n::Text(L"SHADERGRAPH_CREATE_NODE"));
	
	std::map< std::wstring, Ref< ui::MenuItem > > categories;
	for (size_t i = 0; i < sizeof_array(c_nodeCateogories); ++i)
	{
		if (categories.find(c_nodeCateogories[i].category) == categories.end())
		{
			categories[c_nodeCateogories[i].category] = gc_new< ui::MenuItem >(i18n::Text(c_nodeCateogories[i].category));
			menuItemCreate->add(
				categories[c_nodeCateogories[i].category]
			);
		}

		std::wstring title = c_nodeCateogories[i].type.getName();
		size_t p = title.find_last_of(L'.');
		if (p > 0)
			title = i18n::Text(L"SHADERGRAPH_NODE_" + toUpper(title.substr(p + 1)));

		categories[c_nodeCateogories[i].category]->add(
			gc_new< ui::MenuItem >(ui::Command(i, L"ShaderGraph.Editor.Create"), title)
		);
	}

	m_menuPopup->add(menuItemCreate);
	m_menuPopup->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Delete"), i18n::Text(L"SHADERGRAPH_DELETE_NODE")));

	// Setup node facades.
	std::vector< const Type* > nodeTypes;
	Node::getClassType().findAllOf(nodeTypes);

	for (std::vector< const Type* >::const_iterator i = nodeTypes.begin(); i != nodeTypes.end(); ++i)
		m_nodeFacades[*i] = gc_new< DefaultNodeFacade >(m_editorGraph);

	m_nodeFacades[&type_of< Interpolator >()] = gc_new< InterpolatorNodeFacade >();
	m_nodeFacades[&type_of< Switch >()] = gc_new< SwitchNodeFacade >(m_editorGraph);
	m_nodeFacades[&type_of< Swizzle >()] = gc_new< SwizzleNodeFacade >(m_editorGraph);
	m_nodeFacades[&type_of< External >()] = gc_new< ExternalNodeFacade >(m_editorGraph);
	m_nodeFacades[&type_of< Sampler >()] = gc_new< SamplerNodeFacade >(m_editorGraph);

	m_undoStack = gc_new< editor::UndoStack >();
	m_lastValidationResult = true;

	return true;
}

void ShaderGraphEditorPage::destroy()
{
	m_nodeFacades.clear();
	m_editorGraph->destroy();
	m_menuPopup->destroy();
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

	checkUpdatedFragments();

	m_editorGraph->removeAllEdges();
	m_editorGraph->removeAllNodes();

	createEditorNodes(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);

	m_editorGraph->center();

	updateGraph();

	m_editor->setPropertyObject(0);
	m_undoStack = gc_new< editor::UndoStack >();

	m_fragmentGuid = instance->getGuid();

	return true;
}

Object* ShaderGraphEditorPage::getDataObject()
{
	return m_shaderGraph;
}

void ShaderGraphEditorPage::propertiesChanged()
{
	// Save undo state.
	m_undoStack->push(m_shaderGraph);

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

	updateGraph();
}

bool ShaderGraphEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	const Type* primaryType = instance->getPrimaryType();
	T_ASSERT (primaryType);

	// Create sampler node in case of a TextureAsset.
	if (is_type_of< TextureAsset >(*primaryType))
	{
		Ref< Sampler > shaderNode = gc_new< Sampler >(
			instance->getName(),
			instance->getGuid()
		);
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
		Ref< ShaderGraph > fragmentGraph = instance->checkout< ShaderGraph >(db::CfReadOnly);
		T_ASSERT (fragmentGraph);

		Ref< External > shaderNode = gc_new< External >(
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
	if (command == L"Editor.Cut" || command == L"Editor.Copy")
	{
		RefArray< ui::custom::Node > selectedNodes;
		if (m_editorGraph->getSelectedNodes(selectedNodes) > 0)
		{
			// Also copy edges which are affected by selected nodes.
			RefArray< ui::custom::Edge > selectedEdges;
			m_editorGraph->getConnectedEdges(selectedNodes, true, selectedEdges);

			Ref< ShaderGraphEditorClipboardData > data = gc_new< ShaderGraphEditorClipboardData >();
			
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

			ui::Application::getInstance().getClipboard()->setObject(data);

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
			ui::Application::getInstance().getClipboard()->getObject()
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

			m_editor->setPropertyObject(0);
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

			m_editor->setPropertyObject(0);
		}
	}
	else if (command == L"ShaderGraph.Editor.OpenReferee")
	{
		RefereeFilter filter(m_fragmentGuid);
		Ref< db::Instance > refereeInstance = m_editor->browseInstance(&filter);
		if (refereeInstance)
			m_editor->openEditor(refereeInstance);
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
	else if (command == L"ShaderGraph.Editor.Center")
	{
		// Center graph view.
		m_editorGraph->center();
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

		Ref< const OutputPin > shaderSourcePin = shaderEdge->getSource();
		if (!shaderSourcePin)
		{
			log::warning << L"Invalid shader edge, no source pin" << Endl;
			continue;
		}

		Ref< const InputPin > shaderDestinationPin = shaderEdge->getDestination();
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

		Ref< ui::custom::Edge > editorEdge = gc_new< ui::custom::Edge >(editorSourcePin, editorDestinationPin);
		editorEdge->setData(L"SHADEREDGE", shaderEdge);

		m_editorGraph->addEdge(editorEdge);
	}
}

ui::custom::Node* ShaderGraphEditorPage::createEditorNode(Node* shaderNode)
{
	Ref< NodeFacade > nodeFacade = m_nodeFacades[&shaderNode->getType()];
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
		Ref< const InputPin > inputPin = shaderNode->getInputPin(j);
		editorNode->addInputPin(gc_new< ui::custom::Pin >(
			editorNode,
			inputPin->getName(),
			ui::custom::Pin::DrInput,
			true
		));
	}

	for (int j = 0; j < shaderNode->getOutputPinCount(); ++j)
	{
		Ref< const OutputPin > outputPin = shaderNode->getOutputPin(j);
		editorNode->addOutputPin(gc_new< ui::custom::Pin >(
			editorNode,
			outputPin->getName(),
			ui::custom::Pin::DrOutput,
			true
		));
	}

	editorNode->setComment(shaderNode->getComment());

	editorNode->setData(L"SHADERNODE", shaderNode);
	editorNode->setData(L"FACADE", nodeFacade);

	return editorNode;
}

void ShaderGraphEditorPage::createNode(const Type* nodeType, const ui::Point& at)
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

void ShaderGraphEditorPage::checkUpdatedFragments()
{
	bool firstMismatch = true;

	RefArray< External > externalNodes;
	m_shaderGraph->findNodesOf< External >(externalNodes);

	for (RefArray< External >::iterator i = externalNodes.begin(); i != externalNodes.end(); ++i)
	{
		Ref< ShaderGraph > fragmentGraph = m_editor->getSourceDatabase()->getObjectReadOnly< ShaderGraph >((*i)->getFragmentGuid());
		if (!fragmentGraph)
		{
			ui::MessageBox::show(i18n::Text(L"SHADERGRAPH_ERROR_MISSING_FRAGMENT_MESSAGE"), i18n::Text(L"SHADERGRAPH_ERROR_MISSING_FRAGMENT_CAPTION"), ui::MbIconError | ui::MbOk);
			continue;
		}

		RefArray< InputPort > fragmentInputs;
		fragmentGraph->findNodesOf< InputPort >(fragmentInputs);
		RefArray< OutputPort > fragmentOutputs;
		fragmentGraph->findNodesOf< OutputPort >(fragmentOutputs);

		RefArray< InputPin > externalInputPins = (*i)->getInputPins();
		RefArray< OutputPin > externalOutputPins = (*i)->getOutputPins();

		for (RefArray< InputPort >::iterator j = fragmentInputs.begin(); j != fragmentInputs.end(); )
		{
			RefArray< InputPin >::iterator k = externalInputPins.begin();
			while (k != externalInputPins.end())
			{
				if ((*j)->getName() == (*k)->getName())
					break;
				++k;
			}
			if (k != externalInputPins.end())
			{
				j = fragmentInputs.erase(j);
				externalInputPins.erase(k);
			}
			else
				++j;
		}

		for (RefArray< OutputPort >::iterator j = fragmentOutputs.begin(); j != fragmentOutputs.end(); )
		{
			RefArray< OutputPin >::iterator k = externalOutputPins.begin();
			while (k != externalOutputPins.end())
			{
				if ((*j)->getName() == (*k)->getName())
					break;
				++k;
			}
			if (k != externalOutputPins.end())
			{
				j = fragmentOutputs.erase(j);
				externalOutputPins.erase(k);
			}
			else
				++j;
		}

		if (!fragmentInputs.empty() || !fragmentOutputs.empty() || !externalInputPins.empty() || !externalOutputPins.empty())
		{
			if (firstMismatch)
			{
				if (ui::MessageBox::show(m_editorGraph, i18n::Text(L"SHADERGRAPH_EXTERNAL_FRAGMENT_MISMATCH_MESSAGE"), i18n::Text(L"SHADERGRAPH_EXTERNAL_FRAGMENT_MISMATCH_TITLE"), ui::MbIconExclamation | ui::MbYesNo) == ui::DrNo)
					return;
				firstMismatch = false;
			}

			// Remove pins.
			while (!externalInputPins.empty())
			{
				Ref< Edge > edge = ShaderGraphAdjacency(m_shaderGraph).findEdge(externalInputPins.back());
				if (edge)
					m_shaderGraph->removeEdge(edge);

				RefArray< InputPin >::iterator j = std::find((*i)->getInputPins().begin(), (*i)->getInputPins().end(), externalInputPins.back());
				T_ASSERT (j != (*i)->getInputPins().end());
				(*i)->getInputPins().erase(j);

				externalInputPins.pop_back();
			}
			while (!externalOutputPins.empty())
			{
				RefArray< Edge > edges;
				ShaderGraphAdjacency(m_shaderGraph).findEdges(externalOutputPins.back(), edges);
				for (RefArray< Edge >::iterator j = edges.begin(); j != edges.end(); ++j)
					m_shaderGraph->removeEdge(*j);

				RefArray< OutputPin >::iterator j = std::find((*i)->getOutputPins().begin(), (*i)->getOutputPins().end(), externalOutputPins.back());
				T_ASSERT (j != (*i)->getOutputPins().end());
				(*i)->getOutputPins().erase(j);

				externalOutputPins.pop_back();
			}

			// Add new pins.
			for (RefArray< InputPort >::iterator j = fragmentInputs.begin(); j != fragmentInputs.end(); ++j)
				(*i)->getInputPins().push_back(gc_new< InputPin >((*i), (*j)->getName(), false));
			for (RefArray< OutputPort >::iterator j = fragmentOutputs.begin(); j != fragmentOutputs.end(); ++j)
				(*i)->getOutputPins().push_back(gc_new< OutputPin >((*i), (*j)->getName()));
		}
	}
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
		const Type& type = c_nodeCateogories[command.getId()].type;

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

		m_editor->setPropertyObject(shaderNode);
	}
	else
		m_editor->setPropertyObject(0);
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
		m_editor->setPropertyObject(shaderNode);
}

void ShaderGraphEditorPage::eventNodeDoubleClick(ui::Event* event)
{
	Ref< ui::custom::Node > editorNode = checked_type_cast< ui::custom::Node* >(event->getItem());
	Ref< Node > shaderNode = editorNode->getData< Node >(L"SHADERNODE");
	T_ASSERT (shaderNode);

	m_nodeFacades[&shaderNode->getType()]->editShaderNode(
		m_editor,
		m_editorGraph,
		shaderNode
	);

	// Update properties.
	m_editor->setPropertyObject(shaderNode);
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

	Ref< const OutputPin > shaderSourcePin = shaderSourceNode->findOutputPin(editorSourcePin->getName());
	T_ASSERT (shaderSourcePin);

	Ref< const InputPin > shaderDestinationPin = shaderDestinationNode->findInputPin(editorDestinationPin->getName());
	T_ASSERT (shaderDestinationPin);

	// Replace existing edge.
	Ref< Edge > shaderEdge = ShaderGraphAdjacency(m_shaderGraph).findEdge(shaderDestinationPin);
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

	shaderEdge = gc_new< Edge >(shaderSourcePin, shaderDestinationPin);
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
