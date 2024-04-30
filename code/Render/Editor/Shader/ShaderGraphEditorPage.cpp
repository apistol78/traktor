/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/IBrowseFilter.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Editor/PropertiesView.h"
#include "I18N/Text.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/Group.h"
#include "Render/Editor/Shader/FragmentLinker.h"
#include "Render/Editor/Shader/INodeFacade.h"
#include "Render/Editor/Shader/NodeCategories.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderDependencyPane.h"
#include "Render/Editor/Shader/ShaderDependencyTracker.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphEditorClipboardData.h"
#include "Render/Editor/Shader/ShaderGraphEditorPage.h"
#include "Render/Editor/Shader/ShaderViewer.h"
#include "Render/Editor/Shader/QuickMenuTool.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphCombinations.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphEvaluator.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphStatic.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphTechniques.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphValidator.h"
#include "Render/Editor/Shader/Facades/DefaultNodeFacade.h"
#include "Render/Editor/Shader/Facades/ColorNodeFacade.h"
#include "Render/Editor/Shader/Facades/CommentNodeFacade.h"
#include "Render/Editor/Shader/Facades/ExternalNodeFacade.h"
#include "Render/Editor/Shader/Facades/InterpolatorNodeFacade.h"
#include "Render/Editor/Shader/Facades/ScriptNodeFacade.h"
#include "Render/Editor/Shader/Facades/SwizzleNodeFacade.h"
#include "Render/Editor/Shader/Facades/TextureNodeFacade.h"
#include "Render/Editor/Shader/Facades/UniformNodeFacade.h"
#include "Render/Editor/Shader/Facades/VariableNodeFacade.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Command.h"
#include "Ui/Container.h"
#include "Ui/FloodLayout.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/MessageBox.h"
#include "Ui/Splitter.h"
#include "Ui/StyleBitmap.h"
#include "Ui/Tab.h"
#include "Ui/TableLayout.h"
#include "Ui/TabPage.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Group.h"
#include "Ui/Graph/GroupMovedEvent.h"
#include "Ui/Graph/Edge.h"
#include "Ui/Graph/EdgeConnectEvent.h"
#include "Ui/Graph/EdgeDisconnectEvent.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/NodeActivateEvent.h"
#include "Ui/Graph/NodeMovedEvent.h"
#include "Ui/Graph/PaintSettings.h"
#include "Ui/Graph/Pin.h"
#include "Ui/Graph/SelectEvent.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridItemContentChangeEvent.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridRowDoubleClickEvent.h"
#include "Ui/GridView/GridView.h"
#include "Ui/SyntaxRichEdit/SyntaxLanguageGlsl.h"
#include "Ui/SyntaxRichEdit/SyntaxRichEdit.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarDropDown.h"
#include "Ui/ToolBar/ToolBarSeparator.h"

// Resources
#include "Resources/Tools.h"

namespace traktor::render
{
	namespace
	{

const wchar_t* c_pinTypeNames[] =
{
	L"",
	L"Scalar",
	L"Vector 2",
	L"Vector 3",
	L"Vector 4",
	L"Matrix",
	L"Texture 2D",
	L"Texture 3D",
	L"Texture Cube",
	L"Struct Buffer",
	L"Image 2D",
	L"Image 3D",
	L"Image Cube",
	L"State"
};

const wchar_t* c_parameterTypeNames[] =
{
	L"Scalar",
	L"Vector",
	L"Matrix",
	L"Texture 2D",
	L"Texture 3D",
	L"Texture Cube",
	L"Struct Buffer",
	L"Image 2D",
	L"Image 3D",
	L"Image Cube"
};

const wchar_t* c_uniformFrequencyNames[] =
{
	L"Once",
	L"Frame",
	L"Draw"
};

class FragmentReaderAdapter : public FragmentLinker::IFragmentReader
{
public:
	explicit FragmentReaderAdapter(db::Database* db)
	:	m_db(db)
	{
	}

	virtual Ref< const ShaderGraph > read(const Guid& fragmentGuid) const
	{
		return m_db->getObjectReadOnly< ShaderGraph >(fragmentGuid);
	}

private:
	Ref< db::Database > m_db;
};

class EntryCharacter : public RefCountImpl< ui::RichEdit::ISpecialCharacter >
{
public:
	explicit EntryCharacter(Script* script)
	:	m_script(script)
	{
	}

	virtual int32_t measureWidth(const ui::RichEdit* richEdit) const override final
	{
		return richEdit->getFontMetric().getExtent(getEntryString()).cx;
	}

	virtual void draw(ui::Canvas& canvas, const ui::Rect& rc) const override final
	{
		canvas.drawText(rc, getEntryString(), ui::AnCenter, ui::AnCenter);
	}

	virtual void mouseButtonDown(ui::MouseButtonDownEvent* event) const override final {}

	virtual void mouseButtonUp(ui::MouseButtonUpEvent* event) const override final {}

	virtual void mouseDoubleClick(ui::MouseDoubleClickEvent* event) const override final {}

private:
	Script* m_script;

	std::wstring getEntryString() const
	{
		const int32_t inputPinCount = m_script->getInputPinCount();
		const int32_t outputPinCount = m_script->getOutputPinCount();

		StringOutputStream ss;
		ss << L"(";
		for (int32_t i = 0; i < inputPinCount; ++i)
		{
			if (i > 0)
				ss << L", ";
			ss << L"in " << m_script->getInputPin(i)->getName();
		}
		for (int32_t i = 0; i < outputPinCount; ++i)
		{
			if (i > 0 || inputPinCount > 0)
				ss << L", ";
			ss << L"out " << m_script->getOutputPin(i)->getName();
		}
		ss << L")";

		return ss.str();
	}
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphEditorPage", ShaderGraphEditorPage, editor::IEditorPage)

ShaderGraphEditorPage::ShaderGraphEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
{
}

bool ShaderGraphEditorPage::create(ui::Container* parent)
{
	m_shaderGraph = m_document->getObject< ShaderGraph >(0);
	if (!m_shaderGraph)
		return false;

	// Sanitize shader graph; remove broken edges etc.
	int32_t sanitizedCount = 0;
	RefArray< Edge > edges = m_shaderGraph->getEdges();
	for (auto edge : edges)
	{
		const OutputPin* sourcePin = edge->getSource();
		const InputPin* destinationPin = edge->getDestination();
		if (!sourcePin || !destinationPin)
		{
			m_shaderGraph->removeEdge(edge);
			++sanitizedCount;
		}
		else if (sourcePin->getNode() == destinationPin->getNode())
		{
			m_shaderGraph->removeEdge(edge);
			++sanitizedCount;
		}
	}
	if (sanitizedCount > 0)
		log::info << L"Sanitized " << sanitizedCount << L" incorrect data in shader." << Endl;

	m_container = new ui::Container();
	m_container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut));

	// Create our custom toolbar.
	m_toolBar = new ui::ToolBar();
	m_toolBar->create(m_container);
	for (int32_t i = 0; i < 20; ++i)
		m_toolBar->addImage(new ui::StyleBitmap(L"Shader.Tools", i));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_CENTER"), 7, ui::Command(L"ShaderGraph.Editor.Center")));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_ALIGN_LEFT"), 0, ui::Command(L"ShaderGraph.Editor.AlignLeft")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_ALIGN_RIGHT"), 1, ui::Command(L"ShaderGraph.Editor.AlignRight")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_ALIGN_TOP"), 2, ui::Command(L"ShaderGraph.Editor.AlignTop")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_ALIGN_BOTTOM"), 3, ui::Command(L"ShaderGraph.Editor.AlignBottom")));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_EVEN_VERTICALLY"), 4, ui::Command(L"ShaderGraph.Editor.EvenSpaceVertically")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_EVEN_HORIZONTALLY"), 5, ui::Command(L"ShaderGraph.Editor.EventSpaceHorizontally")));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_EVALUATE_CONNECTED"), 14, ui::Command(L"ShaderGraph.Editor.EvaluateConnected")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_EVALUATE_TYPE"), 15, ui::Command(L"ShaderGraph.Editor.EvaluateType")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_REMOVE_UNUSED_NODES"), 8, ui::Command(L"ShaderGraph.Editor.RemoveUnusedNodes")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_AUTO_MERGE_BRANCHES"), 9, ui::Command(L"ShaderGraph.Editor.AutoMergeBranches")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_UPDATE_FRAGMENTS"), 10, ui::Command(L"ShaderGraph.Editor.UpdateFragments")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_CONSTANT_FOLD"), 11, ui::Command(L"ShaderGraph.Editor.ConstantFold")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_SWIZZLED_PERMUTATION"), 12, ui::Command(L"ShaderGraph.Editor.SwizzledPermutation")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_CLEANUP_SWIZZLES"), 18, ui::Command(L"ShaderGraph.Editor.CleanupSwizzles")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_INSERT_INTERPOLATORS"), 13, ui::Command(L"ShaderGraph.Editor.InsertInterpolators")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_RESOLVE_VARIABLES"), 16, ui::Command(L"ShaderGraph.Editor.ResolveVariables")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_RESOLVE_BUNDLES"), 19, ui::Command(L"ShaderGraph.Editor.ResolveBundles")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_RESOLVE_EXTERNALS"), 17, ui::Command(L"ShaderGraph.Editor.ResolveExternals")));

	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolPlatform = new ui::ToolBarDropDown(ui::Command(), 100_ut, i18n::Text(L"SHADERGRAPH_PLATFORM_PERMUTATION"));
	m_toolPlatform->add(L"Android");
	m_toolPlatform->add(L"iOS");
	m_toolPlatform->add(L"Linux");
	m_toolPlatform->add(L"macOS");
	m_toolPlatform->add(L"RaspberryPI");
	m_toolPlatform->add(L"Windows");
	m_toolBar->addItem(m_toolPlatform);
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_PLATFORM_PERMUTATION"), 10, ui::Command(L"ShaderGraph.Editor.PlatformPermutation")));

	m_toolBar->addItem(new ui::ToolBarSeparator());

	m_toolRenderer = new ui::ToolBarDropDown(ui::Command(), 100_ut, i18n::Text(L"SHADERGRAPH_RENDERER_PERMUTATION"));
	m_toolRenderer->add(L"Vulkan");
	m_toolBar->addItem(m_toolRenderer);
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_RENDERER_PERMUTATION"), 10, ui::Command(L"ShaderGraph.Editor.RendererPermutation")));

	m_toolBar->addItem(new ui::ToolBarSeparator());

	m_toolTechniques = new ui::ToolBarDropDown(ui::Command(), 200_ut, i18n::Text(L"SHADERGRAPH_RENDERER_TECHNIQUE"));
	m_toolBar->addItem(m_toolTechniques);
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_RENDERER_TECHNIQUE"), 10, ui::Command(L"ShaderGraph.Editor.Technique")));

	m_toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &ShaderGraphEditorPage::eventToolClick);

	m_scriptSplitter = new ui::Splitter();
	m_scriptSplitter->create(m_container, true, -350_ut, false);

	// Create shader graph editor control.
	m_editorGraph = new ui::GraphControl();
	m_editorGraph->create(m_scriptSplitter);
	m_editorGraph->setText(L"SHADER");
	m_editorGraph->addEventHandler< ui::MouseButtonDownEvent >(this, &ShaderGraphEditorPage::eventButtonDown);
	m_editorGraph->addEventHandler< ui::MouseDoubleClickEvent >(this, &ShaderGraphEditorPage::eventDoubleClick);
	m_editorGraph->addEventHandler< ui::SelectEvent >(this, &ShaderGraphEditorPage::eventSelect);
	m_editorGraph->addEventHandler< ui::GroupMovedEvent >(this, &ShaderGraphEditorPage::eventGroupMoved);
	m_editorGraph->addEventHandler< ui::NodeMovedEvent >(this, &ShaderGraphEditorPage::eventNodeMoved);
	m_editorGraph->addEventHandler< ui::NodeActivateEvent >(this, &ShaderGraphEditorPage::eventNodeDoubleClick);
	m_editorGraph->addEventHandler< ui::EdgeConnectEvent >(this, &ShaderGraphEditorPage::eventEdgeConnect);
	m_editorGraph->addEventHandler< ui::EdgeDisconnectEvent >(this, &ShaderGraphEditorPage::eventEdgeDisconnect);

	// Create script editor control.
	m_scriptEditor = new ui::SyntaxRichEdit();
	m_scriptEditor->create(m_scriptSplitter, L"", ui::WsDoubleBuffer);
	m_scriptEditor->setLanguage(new ui::SyntaxLanguageGlsl());
	m_scriptEditor->hide();
	m_scriptEditor->addEventHandler< ui::ContentChangeEvent >(this, &ShaderGraphEditorPage::eventScriptChange);

	// Load default script editor font from settings.
	const std::wstring font = m_editor->getSettings()->getProperty< std::wstring >(L"Editor.Font", L"Consolas");
	const int32_t fontSize = m_editor->getSettings()->getProperty< int32_t >(L"Editor.FontSize", 11);
	m_scriptEditor->setFont(ui::Font(font, ui::Unit(fontSize)));

	// Create properties view.
	m_propertiesView = m_site->createPropertiesView(parent);
	m_propertiesView->addEventHandler< ui::ContentChangingEvent >(this, &ShaderGraphEditorPage::eventPropertiesChanging);
	m_propertiesView->addEventHandler< ui::ContentChangeEvent >(this, &ShaderGraphEditorPage::eventPropertiesChanged);
	m_site->createAdditionalPanel(m_propertiesView, 400_ut, false);

	// Create shader graph output view.
	m_shaderViewer = new ShaderViewer(m_editor);
	m_shaderViewer->create(parent);
	m_shaderViewer->setVisible(m_editor->getSettings()->getProperty< bool >(L"ShaderEditor.ShaderViewVisible", true));
	m_shaderViewer->setEnable(false);
	m_site->createAdditionalPanel(m_shaderViewer, 400_ut, false);

	// Create "data" view.
	m_dataContainer = new ui::Container();
	m_dataContainer->create(parent, ui::WsNone, new ui::FloodLayout());
	m_dataContainer->setText(i18n::Text(L"SHADERGRAPH_DATA"));
	m_site->createAdditionalPanel(m_dataContainer, 400_ut, false);

	Ref< ui::Tab > tab = new ui::Tab();
	tab->create(m_dataContainer, ui::Tab::WsBottom);

	// Variables tab page.
	Ref< ui::TabPage > tabPageVariables = new ui::TabPage();
	tabPageVariables->create(tab, i18n::Text(L"SHADERGRAPH_VARIABLES"), new ui::FloodLayout());

	m_variablesGrid = new ui::GridView();
	m_variablesGrid->create(tabPageVariables, ui::WsDoubleBuffer | ui::GridView::WsColumnHeader | ui::GridView::WsAutoEdit);
	m_variablesGrid->addColumn(new ui::GridColumn(i18n::Text(L"SHADERGRAPH_VARIABLES_NAME"), 140_ut, true));
	m_variablesGrid->addColumn(new ui::GridColumn(i18n::Text(L"SHADERGRAPH_VARIABLES_N_READ"), 80_ut, false));
	m_variablesGrid->addColumn(new ui::GridColumn(i18n::Text(L"SHADERGRAPH_VARIABLES_TYPE"), 80_ut, false));
	m_variablesGrid->addEventHandler< ui::GridItemContentChangeEvent >(this, &ShaderGraphEditorPage::eventVariableEdit);
	m_variablesGrid->addEventHandler< ui::GridRowDoubleClickEvent >(this, &ShaderGraphEditorPage::eventVariableDoubleClick);

	tab->addPage(tabPageVariables);

	// Uniforms
	Ref< ui::TabPage > tabPageUniforms = new ui::TabPage();
	tabPageUniforms->create(tab, i18n::Text(L"SHADERGRAPH_UNIFORMS"), new ui::FloodLayout());

	m_uniformsGrid = new ui::GridView();
	m_uniformsGrid->create(tabPageUniforms, ui::WsDoubleBuffer | ui::GridView::WsColumnHeader | ui::GridView::WsAutoEdit);
	m_uniformsGrid->setSortColumn(0, false, ui::GridView::SmLexical);
	m_uniformsGrid->addColumn(new ui::GridColumn(i18n::Text(L"SHADERGRAPH_UNIFORMS_NAME"), 190_ut, false));
	m_uniformsGrid->addColumn(new ui::GridColumn(i18n::Text(L"SHADERGRAPH_UNIFORMS_TYPE"), 80_ut, false));
	m_uniformsGrid->addColumn(new ui::GridColumn(i18n::Text(L"SHADERGRAPH_UNIFORMS_FREQUENCY"), 80_ut, false));
	m_uniformsGrid->addEventHandler< ui::GridRowDoubleClickEvent >(this, &ShaderGraphEditorPage::eventUniformOrPortDoubleClick);

	tab->addPage(tabPageUniforms);

	// Ports
	Ref< ui::TabPage > tabPagePorts = new ui::TabPage();
	tabPagePorts->create(tab, i18n::Text(L"SHADERGRAPH_PORTS"), new ui::FloodLayout());

	m_portsGrid = new ui::GridView();
	m_portsGrid->create(tabPagePorts, ui::WsDoubleBuffer | ui::GridView::WsColumnHeader | ui::GridView::WsAutoEdit);
	m_portsGrid->setSortColumn(0, false, ui::GridView::SmLexical);
	m_portsGrid->addColumn(new ui::GridColumn(i18n::Text(L"SHADERGRAPH_PORTS_NAME"), 140_ut, false));
	m_portsGrid->addColumn(new ui::GridColumn(i18n::Text(L"SHADERGRAPH_PORTS_DIRECTION"), 80_ut, false));
	m_portsGrid->addEventHandler< ui::GridRowDoubleClickEvent >(this, &ShaderGraphEditorPage::eventUniformOrPortDoubleClick);

	tab->addPage(tabPagePorts);

	// Node count
	Ref< ui::TabPage > tabPageNodeCount = new ui::TabPage();
	tabPageNodeCount->create(tab, i18n::Text(L"SHADERGRAPH_NODE_COUNTS"), new ui::FloodLayout());

	m_nodeCountGrid = new ui::GridView();
	m_nodeCountGrid->create(tabPageNodeCount, ui::WsDoubleBuffer | ui::GridView::WsColumnHeader | ui::GridView::WsAutoEdit);
	m_nodeCountGrid->setSortColumn(0, false, ui::GridView::SmLexical);
	m_nodeCountGrid->addColumn(new ui::GridColumn(i18n::Text(L"SHADERGRAPH_NODE_TYPE"), 140_ut, false));
	m_nodeCountGrid->addColumn(new ui::GridColumn(i18n::Text(L"SHADERGRAPH_NODE_COUNT"), 80_ut, false));

	tab->addPage(tabPageNodeCount);

	// Shader graph referee tab.
	Ref< ui::TabPage > tabPageReferee = new ui::TabPage();
	tabPageReferee->create(tab, i18n::Text(L"SHADERGRAPH_REFEREES"), new ui::FloodLayout());

	m_dependencyPane = new ShaderDependencyPane(m_editor, m_document->getInstance(0)->getGuid());
	m_dependencyPane->create(tabPageReferee);

	tab->addPage(tabPageReferee);

	tab->setActivePage(tabPageVariables);

	// Build popup menu.
	m_menuPopup = new ui::Menu();
	Ref< ui::MenuItem > menuItemCreate = new ui::MenuItem(i18n::Text(L"SHADERGRAPH_CREATE_NODE"));

	// Collect and create items for each node type.
	std::map< std::wstring, Ref< ui::MenuItem > > categories;
	for (uint32_t i = 0; i < sizeof_array(c_nodeCategories); ++i)
	{
		if (categories.find(c_nodeCategories[i].category) == categories.end())
		{
			categories[c_nodeCategories[i].category] = new ui::MenuItem(i18n::Text(c_nodeCategories[i].category));
			menuItemCreate->add(categories[c_nodeCategories[i].category]);
		}

		std::wstring title = c_nodeCategories[i].type.getName();
		size_t p = title.find_last_of(L'.');
		if (p > 0)
			title = i18n::Text(L"SHADERGRAPH_NODE_" + toUpper(title.substr(p + 1)));

		categories[c_nodeCategories[i].category]->add(
			new ui::MenuItem(ui::Command(i, L"ShaderGraph.Editor.Create"), title)
		);
	}

	// Add favourites.
	Ref< ui::MenuItem > menuItemFavourites = new ui::MenuItem(i18n::Text(L"SHADERGRAPH_CREATE_FAVOURITE"));
	for (const std::wstring& id : m_editor->getSettings()->getProperty< SmallSet< std::wstring > >(L"ShaderEditor.Favourites"))
	{
		Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(Guid(id));
		if (instance)
		{
			Ref< ui::MenuItem > item = new ui::MenuItem(ui::Command(L"ShaderGraph.Editor.CreateFavourite"), instance->getName());
			item->setData(L"INSTANCE", instance);
			menuItemFavourites->add(item);
		}
	}

	m_menuPopup->add(menuItemCreate);
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"ShaderGraph.Editor.CreateGroup"), i18n::Text(L"SHADERGRAPH_CREATE_GROUP")));
	m_menuPopup->add(new ui::MenuItem(L"-"));
	m_menuPopup->add(menuItemFavourites);
	m_menuPopup->add(new ui::MenuItem(L"-"));
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"SHADERGRAPH_DELETE_NODE")));
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"ShaderGraph.Editor.FindInDatabase"), i18n::Text(L"SHADERGRAPH_FIND_IN_DATABASE")));

	// Build quick menu.
	m_menuQuick = new QuickMenuTool();
	m_menuQuick->create(m_editorGraph);

	// Setup node facades.
	for (auto nodeType : type_of< Node >().findAllOf())
		m_nodeFacades[nodeType] = new DefaultNodeFacade();

	m_nodeFacades[&type_of< Color >()] = new ColorNodeFacade();
	m_nodeFacades[&type_of< Comment >()] = new CommentNodeFacade();
	m_nodeFacades[&type_of< External >()] = new ExternalNodeFacade();
	m_nodeFacades[&type_of< Interpolator >()] = new InterpolatorNodeFacade();
	m_nodeFacades[&type_of< Script >()] = new ScriptNodeFacade(this);
	m_nodeFacades[&type_of< Swizzle >()] = new SwizzleNodeFacade();
	m_nodeFacades[&type_of< Texture >()] = new TextureNodeFacade();
	m_nodeFacades[&type_of< Uniform >()] = new UniformNodeFacade();
	m_nodeFacades[&type_of< Variable >()] = new VariableNodeFacade();

	createEditorGraph();

	parent->update();
	m_editorGraph->center();

	updateGraph();
	return true;
}

void ShaderGraphEditorPage::destroy()
{
	if (m_shaderViewer)
	{
		m_editor->checkoutGlobalSettings()->setProperty< PropertyBoolean >(L"ShaderEditor.ShaderViewVisible", m_shaderViewer->isVisible(false));
		m_editor->commitGlobalSettings();
		m_site->destroyAdditionalPanel(m_shaderViewer);
	}

	m_site->destroyAdditionalPanel(m_dataContainer);
	m_site->destroyAdditionalPanel(m_propertiesView);

	m_nodeFacades.clear();
	safeDestroy(m_editorGraph);
	safeDestroy(m_shaderViewer);
	safeDestroy(m_dependencyPane);
	safeDestroy(m_dataContainer);
	safeDestroy(m_propertiesView);
	safeDestroy(m_menuQuick);
}

bool ShaderGraphEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	const TypeInfo* primaryType = instance->getPrimaryType();
	T_ASSERT(primaryType);

	// Create texture node in case of a TextureAsset.
	if (is_type_of< TextureAsset >(*primaryType))
	{
		Ref< Texture > shaderNode = new Texture(instance->getGuid());
		shaderNode->setId(Guid::create());
		m_shaderGraph->addNode(shaderNode);

		const ui::Point absolutePosition = m_editorGraph->screenToClient(position) - m_editorGraph->getOffset();
		shaderNode->setPosition(std::make_pair(absolutePosition.x, absolutePosition.y));

		createEditorNode(shaderNode);

		updateGraph();
	}
	// Create an external node in case of ShaderGraph.
	else if (is_type_of< ShaderGraph >(*primaryType))
	{
		// Prevent dropping itself thus creating cyclic dependencies.
		if (m_document->containInstance(instance))
			return false;

		Ref< ShaderGraph > fragmentGraph = instance->getObject< ShaderGraph >();
		T_ASSERT(fragmentGraph);

		Ref< External > shaderNode = new External(
			instance->getGuid(),
			fragmentGraph
		);
		shaderNode->setId(Guid::create());
		m_shaderGraph->addNode(shaderNode);

		const ui::Point absolutePosition = m_editorGraph->screenToClient(position) - m_editorGraph->getOffset();
		shaderNode->setPosition(std::make_pair(absolutePosition.x, absolutePosition.y));

		createEditorNode(shaderNode);

		updateGraph();
	}
	else
		return false;

	return true;
}

bool ShaderGraphEditorPage::handleCommand(const ui::Command& command)
{
	const RefArray< ui::Node > selectedNodes = m_editorGraph->getSelectedNodes();

	if (m_shaderViewer->handleCommand(command))
		return true;

	if (m_propertiesView->handleCommand(command))
		return true;

	if (!m_scriptEditor->containFocus())
	{
		if (command == L"Editor.Cut" || command == L"Editor.Copy")
		{
			if (!selectedNodes.empty())
			{
				Ref< ShaderGraphEditorClipboardData > data = new ShaderGraphEditorClipboardData();

				for (auto node : selectedNodes)
				{
					Node* shaderNode = node->getData< Node >(L"SHADERNODE");
					T_ASSERT(shaderNode);
					data->addNode(shaderNode);
				}

				for (auto selectedEdge : m_editorGraph->getConnectedEdges(selectedNodes, true))
				{
					Edge* shaderEdge = selectedEdge->getData< Edge >(L"SHADEREDGE");
					T_ASSERT(shaderEdge);
					data->addEdge(shaderEdge);
				}

				ui::Application::getInstance()->getClipboard()->setObject(data);

				// Remove edges and nodes from graphs if user cuts.
				if (command == L"Editor.Cut")
				{
					// Save undo state.
					m_document->push();

					// Remove edges which are connected to any selected node, not only those who connects to both selected end nodes.
					for (auto selectedEdge : m_editorGraph->getConnectedEdges(selectedNodes, false))
					{
						m_shaderGraph->removeEdge(selectedEdge->getData< Edge >(L"SHADEREDGE"));
						m_editorGraph->removeEdge(selectedEdge);
					}

					for (auto selectedNode : selectedNodes)
					{
						m_shaderGraph->removeNode(selectedNode->getData< Node >(L"SHADERNODE"));
						m_editorGraph->removeNode(selectedNode);

						// Ensure script editor is hidden if script node is being cut.
						if (selectedNode->getData< Node >(L"SHADERNODE") == m_script)
							editScript(nullptr);
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
				m_document->push();

				for (auto node : data->getNodes())
				{
					node->setId(Guid::create());
					m_shaderGraph->addNode(node);
				}

				for (auto edge : data->getEdges())
					m_shaderGraph->addEdge(edge);

				RefArray< ui::Node > editorNodes;
				createEditorNodes(
					data->getNodes(),
					data->getEdges(),
					&editorNodes
				);

				// Move all new nodes to center of view.
				ui::UnitRect bounds(65535_ut, 65535_ut, -65535_ut, -65535_ut);
				for (auto node : editorNodes)
				{
					ui::UnitRect rc = node->calculateRect();
					bounds.left = std::min(bounds.left, rc.left);
					bounds.top = std::min(bounds.top, rc.top);
					bounds.right = std::max(bounds.right, rc.right);
					bounds.bottom = std::max(bounds.bottom, rc.bottom);
				}

				const ui::Point mousePosition = m_editorGraph->clientToVirtual(m_editorGraph->getMousePosition());
				const ui::UnitPoint pastePosition = m_editorGraph->unit(mousePosition);

				for (auto node : editorNodes)
				{
					ui::UnitPoint position = node->getPosition();
					position.x = position.x - bounds.left + pastePosition.x - bounds.getWidth() / 2_ut;
					position.y = position.y - bounds.top + pastePosition.y - bounds.getHeight() / 2_ut;
					node->setPosition(position);

					Node* shaderNode = node->getData< Node >(L"SHADERNODE");
					shaderNode->setPosition({ position.x.get(), position.y.get() });
				}

				updateGraph();
			}
		}
		else if (command == L"Editor.SelectAll")
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
			const RefArray< ui::Group > groups = m_editorGraph->getSelectedGroups();

			if (nodes.empty() && groups.empty())
				return false;

			// Save undo state.
			m_document->push();

			// Remove edges first which are connected to selected nodes.
			const RefArray< ui::Edge > edges = m_editorGraph->getConnectedEdges(nodes, false);

			for (auto edge : edges)
			{
				Ref< Edge > shaderEdge = edge->getData< Edge >(L"SHADEREDGE");
				m_editorGraph->removeEdge(edge);
				m_shaderGraph->removeEdge(shaderEdge);
			}

			for (auto node : nodes)
			{
				Ref< Node > shaderNode = node->getData< Node >(L"SHADERNODE");
				m_editorGraph->removeNode(node);
				m_shaderGraph->removeNode(shaderNode);

				// Ensure script editor is hidden if script node is being deleted.
				if (shaderNode == m_script)
					editScript(nullptr);
			}

			for (auto group : groups)
			{
				Ref< Group > shaderGroup = group->getData< Group >(L"SHADERGROUP");
				m_editorGraph->removeGroup(group);
				m_shaderGraph->removeGroup(shaderGroup);
			}

			updateGraph();
		}
		else if (command == L"Editor.Undo")
		{
			if (m_document->undo())
			{
				m_shaderGraph = m_document->getObject< ShaderGraph >(0);
				T_ASSERT(m_shaderGraph);

				createEditorGraph();
			}
		}
		else if (command == L"Editor.Redo")
		{
			if (m_document->redo())
			{
				m_shaderGraph = m_document->getObject< ShaderGraph >(0);
				T_ASSERT(m_shaderGraph);

				createEditorGraph();
			}
		}
		else if (command == L"ShaderGraph.Editor.Center")
		{
			m_editorGraph->center();
		}
		else if (command == L"ShaderGraph.Editor.AlignLeft")
		{
			m_document->push();
			m_editorGraph->alignNodes(ui::GraphControl::AnLeft);
		}
		else if (command == L"ShaderGraph.Editor.AlignRight")
		{
			m_document->push();
			m_editorGraph->alignNodes(ui::GraphControl::AnRight);
		}
		else if (command == L"ShaderGraph.Editor.AlignTop")
		{
			m_document->push();
			m_editorGraph->alignNodes(ui::GraphControl::AnTop);
		}
		else if (command == L"ShaderGraph.Editor.AlignBottom")
		{
			m_document->push();
			m_editorGraph->alignNodes(ui::GraphControl::AnBottom);
		}
		else if (command == L"ShaderGraph.Editor.EvenSpaceVertically")
		{
			m_document->push();
			m_editorGraph->evenSpace(ui::GraphControl::EsVertically);
		}
		else if (command == L"ShaderGraph.Editor.EvenSpaceHorizontally")
		{
			m_document->push();
			m_editorGraph->evenSpace(ui::GraphControl::EsHorizontally);
		}
		else if (command == L"ShaderGraph.Editor.EvaluateConnected")
		{
			m_document->push();

			m_shaderGraph = ShaderGraphStatic(m_shaderGraph, Guid()).getConnectedPermutation();
			T_ASSERT(m_shaderGraph);

			m_document->setObject(0, m_shaderGraph);

			createEditorGraph();
		}
		else if (command == L"ShaderGraph.Editor.EvaluateType")
		{
			m_document->push();

			m_shaderGraph = ShaderGraphStatic(m_shaderGraph, Guid()).getTypePermutation();
			T_ASSERT(m_shaderGraph);

			m_document->setObject(0, m_shaderGraph);

			createEditorGraph();
		}
		else if (command == L"ShaderGraph.Editor.RemoveUnusedNodes")
		{
			m_document->push();

			m_shaderGraph = ShaderGraphOptimizer(m_shaderGraph).removeUnusedBranches(true);
			T_ASSERT(m_shaderGraph);

			m_document->setObject(0, m_shaderGraph);

			createEditorGraph();
		}
		else if (command == L"ShaderGraph.Editor.AutoMergeBranches")
		{
			m_document->push();

			m_shaderGraph = ShaderGraphOptimizer(m_shaderGraph).mergeBranches();
			T_ASSERT(m_shaderGraph);

			m_document->setObject(0, m_shaderGraph);

			createEditorGraph();
		}
		else if (command == L"ShaderGraph.Editor.UpdateFragments")
		{
			const RefArray< ui::Node > selectedNodes = m_editorGraph->getSelectedNodes();

			// Get selected external nodes; ie fragments.
			RefArray< External > selectedExternals;
			for (RefArray< ui::Node >::const_iterator i = selectedNodes.begin(); i != selectedNodes.end(); ++i)
			{
				Ref< External > selectedExternal = (*i)->getData< External >(L"SHADERNODE");
				if (selectedExternal)
					selectedExternals.push_back(selectedExternal);
			}

			if (!selectedExternals.empty())
			{
				m_document->push();

				for (RefArray< External >::const_iterator i = selectedExternals.begin(); i != selectedExternals.end(); ++i)
					updateExternalNode(*i);

				createEditorGraph();
			}
		}
		else if (command == L"ShaderGraph.Editor.ConstantFold")
		{
			m_document->push();

			m_shaderGraph = ShaderGraphStatic(m_shaderGraph, Guid()).getConnectedPermutation();
			T_ASSERT(m_shaderGraph);

			m_shaderGraph = ShaderGraphStatic(m_shaderGraph, Guid()).getTypePermutation();
			T_ASSERT(m_shaderGraph);

			m_shaderGraph = ShaderGraphStatic(m_shaderGraph, Guid()).getConstantFolded();
			T_ASSERT(m_shaderGraph);

			m_document->setObject(0, m_shaderGraph);

			createEditorGraph();
		}
		else if (command == L"ShaderGraph.Editor.SwizzledPermutation")
		{
			m_document->push();

			m_shaderGraph = ShaderGraphStatic(m_shaderGraph, Guid()).getSwizzledPermutation();
			T_ASSERT(m_shaderGraph);

			m_document->setObject(0, m_shaderGraph);

			createEditorGraph();
		}
		else if (command == L"ShaderGraph.Editor.CleanupSwizzles")
		{
			m_document->push();

			m_shaderGraph = ShaderGraphStatic(m_shaderGraph, Guid()).cleanupRedundantSwizzles();
			T_ASSERT(m_shaderGraph);

			m_document->setObject(0, m_shaderGraph);

			createEditorGraph();
		}
		else if (command == L"ShaderGraph.Editor.InsertInterpolators")
		{
			m_document->push();

			m_shaderGraph = ShaderGraphOptimizer(m_shaderGraph).insertInterpolators();
			T_ASSERT(m_shaderGraph);

			m_document->setObject(0, m_shaderGraph);

			createEditorGraph();
		}
		else if (command == L"ShaderGraph.Editor.ResolveVariables")
		{
			m_document->push();

			m_shaderGraph = ShaderGraphStatic(m_shaderGraph, Guid()).getVariableResolved();
			T_ASSERT(m_shaderGraph);

			m_document->setObject(0, m_shaderGraph);

			createEditorGraph();
		}
		else if (command == L"ShaderGraph.Editor.ResolveBundles")
		{
			m_document->push();

			m_shaderGraph = ShaderGraphStatic(m_shaderGraph, Guid()).getBundleResolved();
			T_ASSERT(m_shaderGraph);

			m_document->setObject(0, m_shaderGraph);

			createEditorGraph();
		}
		else if (command == L"ShaderGraph.Editor.ResolveExternals")
		{
			FragmentReaderAdapter reader(m_editor->getSourceDatabase());
			Ref< ShaderGraph > shaderGraph = FragmentLinker(reader).resolve(m_shaderGraph, false);
			if (shaderGraph)
			{
				m_document->push();
				m_shaderGraph = shaderGraph;
				m_document->setObject(0, m_shaderGraph);
				createEditorGraph();
			}
			else
				log::error << L"Fragment linker failed." << Endl;
		}
		else if (command == L"ShaderGraph.Editor.PlatformPermutation")
		{
			m_document->push();

			const std::wstring platformSignature = m_toolPlatform->getSelectedItem();

			m_shaderGraph = ShaderGraphOptimizer(m_shaderGraph).removeUnusedBranches(true);
			T_ASSERT(m_shaderGraph);

			m_shaderGraph = ShaderGraphStatic(m_shaderGraph, Guid()).getPlatformPermutation(platformSignature);
			T_ASSERT(m_shaderGraph);

			m_document->setObject(0, m_shaderGraph);

			createEditorGraph();
		}
		else if (command == L"ShaderGraph.Editor.RendererPermutation")
		{
			m_document->push();

			const std::wstring rendererSignature = m_toolRenderer->getSelectedItem();
			m_shaderGraph = ShaderGraphStatic(m_shaderGraph, Guid()).getRendererPermutation(rendererSignature);
			T_ASSERT(m_shaderGraph);

			m_document->setObject(0, m_shaderGraph);

			createEditorGraph();
		}
		else if (command == L"ShaderGraph.Editor.Technique")
		{
			m_document->push();

			const std::wstring technique = m_toolTechniques->getSelectedItem();
			m_shaderGraph = ShaderGraphTechniques(m_shaderGraph, Guid()).generate(technique);
			T_ASSERT(m_shaderGraph);

			m_document->setObject(0, m_shaderGraph);

			createEditorGraph();
		}
		else if (command == L"ShaderGraph.Editor.QuickMenu")
		{
			const TypeInfo* typeInfo = m_menuQuick->showMenu();
			if (typeInfo)
			{
				m_document->push();

				createNode(
					typeInfo,
					m_editorGraph->clientToVirtual(m_editorGraph->getInnerRect().getCenter())
				);
			}
			m_editorGraph->setFocus();
		}
		else if (command == L"ShaderGraph.Editor.FindInDatabase")
		{
			const RefArray< ui::Node > nodes = m_editorGraph->getSelectedNodes();
			if (nodes.size() != 1)
				return false;

			if (auto selectedExternal = nodes[0]->getData< External >(L"SHADERNODE"))
			{
				Ref< db::Instance > fragmentInstance = m_editor->getSourceDatabase()->getInstance(selectedExternal->getFragmentGuid());
				if (fragmentInstance)
					m_editor->highlightInstance(fragmentInstance);
			}
			else if (auto selectedTexture = nodes[0]->getData< Texture >(L"SHADERNODE"))
			{
				Ref< db::Instance > textureInstance = m_editor->getSourceDatabase()->getInstance(selectedTexture->getExternal());
				if (textureInstance)
					m_editor->highlightInstance(textureInstance);
			}
			else if (auto selectedNode = nodes[0]->getData< Node >(L"SHADERNODE"))
			{
				const Guid selectedNodeId = selectedNode->getId();
				if (selectedNodeId.isNotNull())
				{
					RefArray< db::Instance > shaderGraphInstances;
					db::recursiveFindChildInstances(
						m_editor->getSourceDatabase()->getRootGroup(),
						db::FindInstanceByType(type_of< ShaderGraph >()),
						shaderGraphInstances
					);
					for (auto shaderGraphInstance : shaderGraphInstances)
					{
						auto shaderGraph = shaderGraphInstance->getObject< ShaderGraph >();
						if (!shaderGraph)
							continue;

						for (auto node : shaderGraph->getNodes())
						{
							if (node->getId() == selectedNodeId)
								log::info << L"Found node in " << shaderGraphInstance->getGuid().format() << Endl;
						}
					}
				}
			}
		}
		else
			return false;
	}
	else
	{
		if (command == L"Editor.Undo")
		{
			if (m_document->undo())
			{
				m_shaderGraph = m_document->getObject< ShaderGraph >(0);
				T_ASSERT(m_shaderGraph);

				createEditorGraph();
			}
		}
		else if (command == L"Editor.Redo")
		{
			if (m_document->redo())
			{
				m_shaderGraph = m_document->getObject< ShaderGraph >(0);
				T_ASSERT(m_shaderGraph);

				createEditorGraph();
			}
		}
		else
			return false;
	}

	m_editorGraph->update();
	return true;
}

void ShaderGraphEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	if (m_shaderGraph)
		m_shaderViewer->reflect(m_shaderGraph);
}

void ShaderGraphEditorPage::editScript(Script* script)
{
	if ((m_script = script) != nullptr)
	{
		const ui::Size szInner = m_container->getInnerRect().getSize();
		m_scriptSplitter->setVertical(szInner.cx >= szInner.cy);

		const wchar_t sc = m_scriptEditor->addSpecialCharacter(new EntryCharacter(m_script));

		std::wstring text = m_script->getScript();
		text = replaceAll(text, L"ENTRY", std::wstring(1, sc));

		m_scriptEditor->setText(text);
		m_scriptEditor->show();
	}
	else
	{
		m_scriptEditor->clear(false, true, true, true);
		m_scriptEditor->hide();
	}

	m_scriptEditor->getParent()->update();
	m_scriptEditor->update();
}

void ShaderGraphEditorPage::createEditorGraph()
{
	const Guid scriptId = m_script ? m_script->getId() : Guid();

	editScript(nullptr);

	m_editorGraph->removeAllGroups();
	m_editorGraph->removeAllNodes();
	m_editorGraph->removeAllEdges();

	createEditorNodes(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges(),
		nullptr
	);

	for (auto group : m_shaderGraph->getGroups())
		createEditorGroup(group);

	updateGraph();

	if (scriptId.isNotNull())
	{
		for (auto node : m_shaderGraph->getNodes())
		{
			if (node->getId() == scriptId)
				editScript(dynamic_type_cast< Script* >(node));
		}
	}	

	m_propertiesView->setPropertyObject(nullptr);
}

void ShaderGraphEditorPage::createEditorNodes(const RefArray< Node >& shaderNodes, const RefArray< Edge >& shaderEdges, RefArray< ui::Node >* outEditorNodes)
{
	// Keep a map from shader nodes to editor nodes.
	std::map< Ref< Node >, Ref< ui::Node > > nodeMap;

	// Create editor nodes for each shader node.
	for (auto shaderNode : shaderNodes)
	{
		Ref< ui::Node > editorNode = createEditorNode(shaderNode);
		nodeMap[shaderNode] = editorNode;
		if (outEditorNodes)
			outEditorNodes->push_back(editorNode);
	}

	// Create editor edges for each shader edge.
	for (auto shaderEdge : shaderEdges)
	{
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

		Ref< ui::Node > editorSourceNode = nodeMap[shaderSourcePin->getNode()];
		if (!editorSourceNode)
		{
			log::warning << L"Invalid shader pin, no editor source node found of pin \"" << shaderSourcePin->getName() << L"\"" << Endl;
			continue;
		}

		Ref< ui::Node > editorDestinationNode = nodeMap[shaderDestinationPin->getNode()];
		if (!editorDestinationNode)
		{
			log::warning << L"Invalid shader pin, no editor destination node found of pin \"" << shaderDestinationPin->getName() << L"\"" << Endl;
			continue;
		}

		Ref< ui::Pin > editorSourcePin = editorSourceNode->findOutputPin(shaderSourcePin->getName());
		if (!editorSourcePin)
		{
			log::warning << L"Unable to find editor source pin \"" << shaderSourcePin->getName() << L"\"" << Endl;
			continue;
		}

		Ref< ui::Pin > editorDestinationPin = editorDestinationNode->findInputPin(shaderDestinationPin->getName());
		if (!editorDestinationPin)
		{
			log::warning << L"Unable to find editor destination pin \"" << shaderDestinationPin->getName() << L"\"" << Endl;
			continue;
		}

		Ref< ui::Edge > editorEdge = new ui::Edge(editorSourcePin, editorDestinationPin);
		editorEdge->setData(L"SHADEREDGE", shaderEdge);

		m_editorGraph->addEdge(editorEdge);
	}
}

Ref< ui::Node > ShaderGraphEditorPage::createEditorNode(Node* shaderNode)
{
	Ref< INodeFacade > nodeFacade = m_nodeFacades[&type_of(shaderNode)];
	T_ASSERT_M (nodeFacade, L"No node facade class found");

	Ref< ui::Node > editorNode = nodeFacade->createEditorNode(
		m_editor,
		m_editorGraph,
		m_shaderGraph,
		shaderNode
	);

	if (!editorNode)
		return nullptr;

	editorNode->setData(L"SHADERNODE", shaderNode);
	editorNode->setData(L"FACADE", nodeFacade);

	return editorNode;
}

Ref< ui::Group > ShaderGraphEditorPage::createEditorGroup(Group* shaderGroup)
{
	const auto p = shaderGroup->getPosition();
	const auto s = shaderGroup->getSize();

	Ref< ui::Group > editorGroup = m_editorGraph->createGroup(
		shaderGroup->getTitle(),
		ui::UnitPoint(ui::Unit(p.first), ui::Unit(p.second)),
		ui::UnitSize(ui::Unit(s.first), ui::Unit(s.second))
	);
	editorGroup->setData(L"SHADERGROUP", shaderGroup);
	return editorGroup;
}

void ShaderGraphEditorPage::createNode(const TypeInfo* nodeType, const ui::Point& at)
{
	Ref< Node > shaderNode = m_nodeFacades[nodeType]->createShaderNode(nodeType, m_editor);
	if (!shaderNode)
		return;

	const ui::UnitPoint uat = m_editorGraph->unit(at);

	// Add to shader graph.
	shaderNode->setId(Guid::create());
	shaderNode->setPosition({ uat.x.get(), uat.y.get() });
	m_shaderGraph->addNode(shaderNode);

	// Create editor node from shader node.
	createEditorNode(shaderNode);
	updateGraph();
}

void ShaderGraphEditorPage::refreshGraph()
{
	// Refresh editor nodes.
	for (auto editorNode : m_editorGraph->getNodes())
	{
		Node* shaderNode = editorNode->getData< Node >(L"SHADERNODE");
		INodeFacade* nodeFacade = editorNode->getData< INodeFacade >(L"FACADE");
		nodeFacade->refreshEditorNode(m_editor, m_editorGraph, editorNode, m_shaderGraph, shaderNode);
	}

	// Refresh editor groups.
	for (auto editorGroup : m_editorGraph->getGroups())
	{
		Group* shaderGroup = editorGroup->getData< Group >(L"SHADERGROUP");
		editorGroup->setTitle(shaderGroup->getTitle());
	}
}

void ShaderGraphEditorPage::updateGraph()
{
	struct VariableInfo
	{
		uint32_t count = 0;
		uint32_t writeCount = 0;
		uint32_t readCount = 0;
		PinType type = PinType::Void;
	};
	std::map< std::wstring, VariableInfo > variables;

	// Fully resolve shader graph so we can inspect all uniforms etc.
	FragmentReaderAdapter reader(m_editor->getSourceDatabase());
	Ref< ShaderGraph > resolvedShaderGraph = FragmentLinker(reader).resolve(m_shaderGraph, true);

	// Extract techniques.
	m_toolTechniques->removeAll();	
	for (const auto& technique : ShaderGraphTechniques(m_shaderGraph, Guid()).getNames())
		m_toolTechniques->add(technique);

	// Update variables grid.
	for (auto variableNode : m_shaderGraph->findNodesOf< Variable >())
	{
		auto& vi = variables[variableNode->getName()];

		vi.count++;
		vi.readCount += m_shaderGraph->getDestinationCount(variableNode->getOutputPin(0));

		const Edge* sourceEdge = m_shaderGraph->findEdge(variableNode->getInputPin(0));
		if (sourceEdge != nullptr)
		{
			++vi.writeCount;

			Constant value = ShaderGraphEvaluator(m_shaderGraph).evaluate(sourceEdge->getSource());
			vi.type = value.getType();
		}
	}
	m_variablesGrid->removeAllRows();
	for (const auto& variable : variables)
	{
		Ref< ui::GridRow > row = new ui::GridRow();
		row->add(variable.first);

		if (variable.second.readCount > 0 && variable.second.writeCount == 0)
			row->setBackground(Color4ub(255, 0, 0, 255));

		row->add(toString(variable.second.readCount));
		row->add(c_pinTypeNames[(int32_t)variable.second.type]);
		m_variablesGrid->addRow(row);
	}

	// Update uniforms grid.
	m_uniformsGrid->removeAllRows();
	for (auto node : m_shaderGraph->getNodes())
	{
		if (Uniform* uniformNode = dynamic_type_cast< Uniform* >(node))
		{
			Ref< ui::GridRow > row = new ui::GridRow();
			row->setData(L"SHADERNODE", uniformNode);
			row->add(uniformNode->getParameterName());
			row->add(c_parameterTypeNames[(int32_t)uniformNode->getParameterType()]);
			row->add(c_uniformFrequencyNames[(int32_t)uniformNode->getFrequency()]);
			m_uniformsGrid->addRow(row);
		}
		else if (IndexedUniform* indexedUniformNode = dynamic_type_cast< IndexedUniform* >(node))
		{
			Ref< ui::GridRow > row = new ui::GridRow();
			row->setData(L"SHADERNODE", indexedUniformNode);
			row->add(indexedUniformNode->getParameterName());
			row->add(c_parameterTypeNames[(int32_t)indexedUniformNode->getParameterType()]);
			row->add(c_uniformFrequencyNames[(int32_t)indexedUniformNode->getFrequency()]);
			m_uniformsGrid->addRow(row);
		}
	}
	if (resolvedShaderGraph)
	{
		// Add non-local uniforms to grid as well, color coded.
		for (auto node : resolvedShaderGraph->getNodes())
		{
			auto it = std::find_if(
				m_shaderGraph->getNodes().begin(), m_shaderGraph->getNodes().end(),
				[&](const Node* n) {
					return node->getId() == n->getId();
				}
			);
			if (it != m_shaderGraph->getNodes().end())
				continue;

			if (Uniform* uniformNode = dynamic_type_cast< Uniform* >(node))
			{
				Ref< ui::GridRow > row = new ui::GridRow();
				row->add(uniformNode->getParameterName());
				row->add(c_parameterTypeNames[(int32_t)uniformNode->getParameterType()]);
				row->add(c_uniformFrequencyNames[(int32_t)uniformNode->getFrequency()]);
				row->setBackground(Color4ub(90, 60, 40, 255));
				m_uniformsGrid->addRow(row);
			}
			else if (IndexedUniform* indexedUniformNode = dynamic_type_cast< IndexedUniform* >(node))
			{
				Ref< ui::GridRow > row = new ui::GridRow();
				row->add(indexedUniformNode->getParameterName());
				row->add(c_parameterTypeNames[(int32_t)indexedUniformNode->getParameterType()]);
				row->add(c_uniformFrequencyNames[(int32_t)indexedUniformNode->getFrequency()]);
				row->setBackground(Color4ub(90, 60, 40, 255));
				m_uniformsGrid->addRow(row);
			}
		}
	}

	// Update ports grid.
	m_portsGrid->removeAllRows();
	for (auto node : m_shaderGraph->getNodes())
	{
		if (InputPort* inputPortNode = dynamic_type_cast< InputPort* >(node))
		{
			Ref< ui::GridRow > row = new ui::GridRow();
			row->setData(L"SHADERNODE", inputPortNode);
			row->add(inputPortNode->getName());
			row->add(L"Input");
			m_portsGrid->addRow(row);
		}
		else if (OutputPort* outputPortNode = dynamic_type_cast< OutputPort* >(node))
		{
			Ref< ui::GridRow > row = new ui::GridRow();
			row->setData(L"SHADERNODE", outputPortNode);
			row->add(outputPortNode->getName());
			row->add(L"Output");
			m_portsGrid->addRow(row);
		}
	}

	// Update node count grid.
	m_nodeCountGrid->removeAllRows();

	SmallMap< const TypeInfo*, int32_t > nodeCounts;
	for (auto node : m_shaderGraph->getNodes())
		nodeCounts[&type_of(node)]++;

	for (const auto& it : nodeCounts)
	{
		Ref< ui::GridRow > row = new ui::GridRow();
		row->add(it.first->getName());
		row->add(str(L"%d", it.second));
		m_nodeCountGrid->addRow(row);
	}

	// Determine type of shader graph.
	const auto graphType = resolvedShaderGraph ? ShaderGraphValidator(resolvedShaderGraph).estimateType() : ShaderGraphValidator::SgtFragment;
	m_editorGraph->setText(graphType == ShaderGraphValidator::SgtFragment ? L"FRAGMENT" : L"SHADER");
	
	// Validate shader graph.
	AlignedVector< const Node* > errorNodes;
	const ShaderGraphValidator validator(m_shaderGraph);
	const bool validationResult = validator.validate(graphType, &errorNodes);

	// Update validation status of each node.
	for (auto editorNode : m_editorGraph->getNodes())
	{
		Ref< Node > shaderNode = editorNode->getData< Node >(L"SHADERNODE");
		T_ASSERT(shaderNode);

		Ref< INodeFacade > nodeFacade = editorNode->getData< INodeFacade >(L"FACADE");
		T_ASSERT(nodeFacade);

		if (std::find(errorNodes.begin(), errorNodes.end(), shaderNode) != errorNodes.end())
			nodeFacade->setValidationIndicator(editorNode, false);
		else
			nodeFacade->setValidationIndicator(editorNode, true);
	}

	// If validation succeeded then update generated shader as well.
	m_shaderViewer->setEnable(graphType == ShaderGraphValidator::SgtProgram);
	if (validationResult && graphType == ShaderGraphValidator::SgtProgram)
		m_shaderViewer->reflect(m_shaderGraph);

	// Evaluate output types (and partial values) if validation succeeded.
	if (validationResult && resolvedShaderGraph)
	{
		do
		{
			// Prepare evaluation graph, ie graph which contain less meta nodes so evaluator can process graph properly.
			Ref< ShaderGraph > evaluationGraph = ShaderGraphStatic(resolvedShaderGraph, Guid()).getVariableResolved();
			if (!evaluationGraph)
				break;
			evaluationGraph = ShaderGraphStatic(evaluationGraph, Guid()).getConnectedPermutation();
			if (!evaluationGraph)
				break;
			evaluationGraph = ShaderGraphStatic(evaluationGraph, Guid()).getTypePermutation();
			if (!evaluationGraph)
				break;

			const ShaderGraphEvaluator evaluator(evaluationGraph);
			for (auto editorEdge : m_editorGraph->getEdges())
			{
				Ref< Edge > shaderEdge = editorEdge->getData< Edge >(L"SHADEREDGE");
				T_ASSERT(shaderEdge);

				Constant value;
				for (auto edge : evaluationGraph->getEdges())
				{
					if (
						(
							shaderEdge->getSource()->getNode()->getId() == edge->getSource()->getNode()->getId() &&
							shaderEdge->getSource()->getId() == edge->getSource()->getId()
						) ||
						(
							shaderEdge->getDestination()->getNode()->getId() == edge->getDestination()->getNode()->getId() &&
							shaderEdge->getDestination()->getId() == edge->getDestination()->getId()
						)
					)
					{
						value = evaluator.evaluate(edge->getSource());
						break;
					}
				}

				// Set default thickness first; override below for "fat" types.
				editorEdge->setThickness(2_ut);

				StringOutputStream ss;
				switch (value.getType())
				{
				default:
				case PinType::Void:
					break;

				case PinType::Scalar1:
				case PinType::Scalar2:
				case PinType::Scalar3:
				case PinType::Scalar4:
					for (int32_t i = 0; i < value.getWidth(); ++i)
					{
						if (i > 0)
							ss << L",";

						if (value.isConst(i))
							ss << value.getValue(i);
						else
							ss << L"X";
					}
					break;

				case PinType::Matrix:
					ss << L"Matrix";
					break;

				case PinType::Texture2D:
					ss << L"Texture2d";
					editorEdge->setThickness(4_ut);
					break;

				case PinType::Texture3D:
					ss << L"Texture3d";
					editorEdge->setThickness(4_ut);
					break;

				case PinType::TextureCube:
					ss << L"TextureCube";
					editorEdge->setThickness(4_ut);
					break;

				case PinType::StructBuffer:
					ss << L"StructBuffer";
					editorEdge->setThickness(4_ut);
					break;

				case PinType::Image2D:
					ss << L"Image2d";
					editorEdge->setThickness(4_ut);
					break;

				case PinType::Image3D:
					ss << L"Image3d";
					editorEdge->setThickness(4_ut);
					break;

				case PinType::ImageCube:
					ss << L"ImageCube";
					editorEdge->setThickness(4_ut);
					break;

				case PinType::State:
					ss << L"State";
					editorEdge->setThickness(4_ut);
					break;

				case PinType::Bundle:
					ss << L"Bundle";
					editorEdge->setThickness(6_ut);
					break;
				}

				editorEdge->setText(ss.str());
			}
		}
		while (false);
	}
	else
	{
		for (auto editorEdge : m_editorGraph->getEdges())
		{
			editorEdge->setText(L"");
			editorEdge->setThickness(2_ut);
		}
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
	RefArray< InputPort > fragmentInputs = fragmentGraph->findNodesOf< InputPort >();

	RefArray< InputPort >::iterator it;
	it = std::remove_if(fragmentInputs.begin(), fragmentInputs.end(), [](InputPort* ip) {
		return ip->isConnectable() == false && ip->isOptional() == false;
	});
	fragmentInputs.erase(it, fragmentInputs.end());
	it = std::remove_if(fragmentInputs.begin(), fragmentInputs.end(), [](InputPort* ip) {
		return ip->isConnectable() == false && ip->isOptional() == true;
	});
	fragmentInputs.erase(it, fragmentInputs.end());

	// Get output ports.
	RefArray< OutputPort > fragmentOutputs = fragmentGraph->findNodesOf< OutputPort >();

	// Get input-/output pins; these might differ if fragment has been updated.
	const uint32_t externalInputPinCount = external->getInputPinCount();
	const uint32_t externalOutputPinCount = external->getOutputPinCount();

	std::vector< const InputPin* > externalInputPins(externalInputPinCount);
	for (uint32_t i = 0; i < externalInputPinCount; ++i)
		externalInputPins[i] = external->getInputPin(i);

	std::vector< const OutputPin* > externalOutputPins(externalOutputPinCount);
	for (uint32_t i = 0; i < externalOutputPinCount; ++i)
		externalOutputPins[i] = external->getOutputPin(i);

	// Remove input ports and pins which match.
	for (RefArray< InputPort >::iterator i = fragmentInputs.begin(); i != fragmentInputs.end(); )
	{
		auto j = std::find_if(externalInputPins.begin(), externalInputPins.end(), [&](const InputPin* externalInputPin) {
			return
				externalInputPin->getName() == (*i)->getName() &&
				externalInputPin->isOptional() == (*i)->isOptional();
		});
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
		auto j = std::find_if(externalOutputPins.begin(), externalOutputPins.end(), [&](const OutputPin* externalOutputPin) {
			return externalOutputPin->getName() == (*i)->getName();
		});
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
		for (auto edge : m_shaderGraph->findEdges(externalOutputPins.back()))
			m_shaderGraph->removeEdge(edge);

		external->removeOutputPin(externalOutputPins.back());
		externalOutputPins.pop_back();
	}

	// Add new pins for new ports.
	for (const auto& inputPort : fragmentInputs)
		external->createInputPin(inputPort->getId(), inputPort->getName(), inputPort->isOptional());
	for (const auto& outputPort : fragmentOutputs)
		external->createOutputPin(outputPort->getId(), outputPort->getName());
}

void ShaderGraphEditorPage::updateVariableHints()
{
	m_editorGraph->removeAllDependencyHints();

	RefArray< ui::Node > variableNodes;
	RefArray< ui::Node > selectedVariableNodes;
	for (auto editorNode : m_editorGraph->getNodes())
	{
		if (editorNode->getData< Variable >(L"SHADERNODE") != nullptr)
		{
			variableNodes.push_back(editorNode);
			if (editorNode->isSelected())
				selectedVariableNodes.push_back(editorNode);
		}
	}

	for (auto selectVariableNode : selectedVariableNodes)
	{
		for (auto variableNode : variableNodes)
		{
			if (variableNode->getInfo() == selectVariableNode->getInfo())
				m_editorGraph->addDependencyHint(variableNode, selectVariableNode);
		}
	}

	m_editorGraph->update();
}

void ShaderGraphEditorPage::eventToolClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	handleCommand(command);
}

void ShaderGraphEditorPage::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	const ui::Point position = m_editorGraph->clientToVirtual(event->getPosition());

	if (event->getButton() == ui::MbtLeft)
	{
		// Override default graph behaviour and instead create value nodes.
		if (ui::Application::getInstance()->getEventLoop()->isKeyDown(ui::VkS))
		{
			createNode(&type_of< Scalar >(), position);
			event->consume();
		}
		else if (ui::Application::getInstance()->getEventLoop()->isKeyDown(ui::VkC))
		{
			createNode(&type_of< Color >(), position);
			event->consume();
		}
		else if (ui::Application::getInstance()->getEventLoop()->isKeyDown(ui::VkV))
		{
			createNode(&type_of< Vector >(), position);
			event->consume();
		}
	}
	else if (event->getButton() == ui::MbtRight)
	{
		const ui::MenuItem* selected = m_menuPopup->showModal(m_editorGraph, event->getPosition());
		if (!selected)
			return;

		const ui::Command& command = selected->getCommand();

		if (command == L"ShaderGraph.Editor.Create")	// Create node
		{
			m_document->push();
			createNode(&c_nodeCategories[command.getId()].type, position);
		}
		else if (command == L"ShaderGraph.Editor.CreateGroup")	// Create group.
		{
			m_document->push();

			// Add to shader graph.
			Ref< Group > shaderGroup = new Group();
			shaderGroup->setPosition({ position.x, position.y });
			shaderGroup->setSize({ 200, 200 });
			m_shaderGraph->addGroup(shaderGroup);

			// Create editor group from shader group.
			createEditorGroup(shaderGroup);

			updateGraph();
		}
		else if (command == L"ShaderGraph.Editor.CreateFavourite")	// Create favourite fragment node.
		{
			Ref< db::Instance > instance = selected->getData< db::Instance >(L"INSTANCE");
			T_FATAL_ASSERT(instance != nullptr);

			// Prevent dropping itself thus creating cyclic dependencies.
			if (!m_document->containInstance(instance))
			{
				Ref< ShaderGraph > fragmentGraph = instance->getObject< ShaderGraph >();
				T_ASSERT(fragmentGraph);

				// Add to shader graph.
				Ref< External > shaderNode = new External(
					instance->getGuid(),
					fragmentGraph
				);
				shaderNode->setId(Guid::create());
				shaderNode->setPosition({ position.x, position.y });
				m_shaderGraph->addNode(shaderNode);

				// Create editor node from shader node.
				createEditorNode(shaderNode);
				updateGraph();
			}
		}
		else
			handleCommand(command);
	}
}

void ShaderGraphEditorPage::eventDoubleClick(ui::MouseDoubleClickEvent* event)
{
	if (m_editorGraph->getSelectedNodes().empty())
		editScript(nullptr);
}

void ShaderGraphEditorPage::eventSelect(ui::SelectEvent* event)
{
	const RefArray< ui::Node > nodes = m_editorGraph->getSelectedNodes();
	const RefArray< ui::Group > groups = m_editorGraph->getSelectedGroups();

	if (nodes.size() == 1)
	{
		Ref< Node > shaderNode = nodes.front()->getData< Node >(L"SHADERNODE");
		m_propertiesView->setPropertyObject(shaderNode);
	}
	else if (groups.size() == 1)
	{
		Ref< Group > shaderGroup = groups.front()->getData< Group >(L"SHADERGROUP");
		m_propertiesView->setPropertyObject(shaderGroup);
	}
	else
		m_propertiesView->setPropertyObject(nullptr);

	updateVariableHints();
}

void ShaderGraphEditorPage::eventGroupMoved(ui::GroupMovedEvent* event)
{
	Ref< ui::Group > editorGroup = event->getGroup();
	T_ASSERT(editorGroup);

	// Get shader graph node from editor node.
	Ref< Group > shaderGroup = editorGroup->getData< Group >(L"SHADERGROUP");
	T_ASSERT(shaderGroup);

	const ui::UnitRect rc = editorGroup->calculateRect();

	//if (
	//	rc.left != shaderGroup->getPosition().first || rc.top != shaderGroup->getPosition().second ||
	//	rc.left != shaderGroup->getPosition().first || rc.top != shaderGroup->getPosition().second
	//)
	{
		//m_document->push();
		shaderGroup->setPosition({ rc.left.get(), rc.top.get() });
		shaderGroup->setSize({ rc.getSize().cx.get(), rc.getSize().cy.get() });
	}
}

void ShaderGraphEditorPage::eventNodeMoved(ui::NodeMovedEvent* event)
{
	Ref< ui::Node > editorNode = event->getNode();
	T_ASSERT(editorNode);

	// Get shader graph node from editor node.
	Ref< Node > shaderNode = editorNode->getData< Node >(L"SHADERNODE");
	T_ASSERT(shaderNode);

	const ui::UnitPoint position = editorNode->getPosition();

	if (position.x.get() != shaderNode->getPosition().first || position.y.get() != shaderNode->getPosition().second)
	{
		m_document->push();
		shaderNode->setPosition({ position.x.get(), position.y.get() });
	}
}

void ShaderGraphEditorPage::eventNodeDoubleClick(ui::NodeActivateEvent* event)
{
	Ref< ui::Node > editorNode = event->getNode();
	Ref< Node > shaderNode = editorNode->getData< Node >(L"SHADERNODE");
	T_ASSERT(shaderNode);

	// Update properties.
	m_propertiesView->setPropertyObject(shaderNode);

	// Edit node.
	m_nodeFacades[&type_of(shaderNode)]->editShaderNode(
		m_editor,
		m_editorGraph,
		editorNode,
		m_shaderGraph,
		shaderNode
	);

	// Update properties.
	m_propertiesView->setPropertyObject(shaderNode);

	// Refresh graph; information might have changed.
	refreshGraph();
}

void ShaderGraphEditorPage::eventEdgeConnect(ui::EdgeConnectEvent* event)
{
	Ref< ui::Edge > editorEdge = event->getEdge();
	Ref< ui::Pin > editorSourcePin = editorEdge->getSourcePin();
	T_ASSERT(editorSourcePin);

	Ref< ui::Pin > editorDestinationPin = editorEdge->getDestinationPin();
	T_ASSERT(editorDestinationPin);

	Ref< Node > shaderSourceNode = editorSourcePin->getNode()->getData< Node >(L"SHADERNODE");
	T_ASSERT(shaderSourceNode);

	Ref< Node > shaderDestinationNode = editorDestinationPin->getNode()->getData< Node >(L"SHADERNODE");
	T_ASSERT(shaderDestinationNode);

	const OutputPin* shaderSourcePin = shaderSourceNode->findOutputPin(editorSourcePin->getName());
	T_ASSERT(shaderSourcePin);

	const InputPin* shaderDestinationPin = shaderDestinationNode->findInputPin(editorDestinationPin->getName());
	T_ASSERT(shaderDestinationPin);

	// Replace existing edge.
	Ref< Edge > shaderEdge = m_shaderGraph->findEdge(shaderDestinationPin);
	if (shaderEdge)
	{
		m_shaderGraph->removeEdge(shaderEdge);

		RefArray< ui::Edge > editorEdges = m_editorGraph->getConnectedEdges(editorDestinationPin);
		if (editorEdges.size() > 0)
			m_editorGraph->removeEdge(editorEdges.front());
	}

	m_document->push();

	shaderEdge = new Edge(shaderSourcePin, shaderDestinationPin);
	m_shaderGraph->addEdge(shaderEdge);

	editorEdge->setData(L"SHADEREDGE", shaderEdge);
	m_editorGraph->addEdge(editorEdge);

	updateGraph();
}

void ShaderGraphEditorPage::eventEdgeDisconnect(ui::EdgeDisconnectEvent* event)
{
	ui::Edge* editorEdge = event->getEdge();
	Edge* edge = mandatory_non_null_type_cast< Edge* >(editorEdge->getData(L"SHADEREDGE"));

	m_document->push();
	m_shaderGraph->removeEdge(edge);

	updateGraph();
}

void ShaderGraphEditorPage::eventScriptChange(ui::ContentChangeEvent* event)
{
	T_FATAL_ASSERT(m_script);

	m_document->push();

	// Transform editor text into "escaped" text.
	std::wstring text = m_scriptEditor->getText(
		[&] (wchar_t ch) -> std::wstring {
			return ch != L'\\' ? std::wstring(1, ch) : L"\\\\";
		},
		[&] (const ui::RichEdit::ISpecialCharacter* sc) -> std::wstring {
			const EntryCharacter* dc = static_cast< const EntryCharacter* >(sc);
			return L"ENTRY";
		}
	);

	m_script->setScript(text);
}

void ShaderGraphEditorPage::eventPropertiesChanging(ui::ContentChangingEvent* event)
{
	m_document->push();
}

void ShaderGraphEditorPage::eventPropertiesChanged(ui::ContentChangeEvent* event)
{
	refreshGraph();
	updateGraph();
}

void ShaderGraphEditorPage::eventVariableEdit(ui::GridItemContentChangeEvent* event)
{
	RefArray< Variable > variableNodes = m_shaderGraph->findNodesOf< Variable >();

	const std::wstring renameFrom = event->getOriginalText();
	const std::wstring renameTo = event->getItem()->getText();

	if (renameFrom == renameTo)
		return;

	// Check if "rename to" is a valid name, i.e. not empty nor collide.
	if (renameTo.empty())
		return;

	for (auto variableNode : variableNodes)
	{
		const std::wstring name = variableNode->getName();
		if (name != renameFrom && name == renameTo)
			return;
	}

	// Name is valid, rename variables.
	m_document->push();
	for (auto variableNode : variableNodes)
	{
		const std::wstring name = variableNode->getName();
		if (name == renameFrom)
			variableNode->setName(renameTo);
	}

	refreshGraph();

	m_editorGraph->update();
	event->consume();
}

void ShaderGraphEditorPage::eventVariableDoubleClick(ui::GridRowDoubleClickEvent* event)
{
	std::wstring variableName = event->getRow()->get(0)->getText();

	RefArray< Variable > variableNodes = m_shaderGraph->findNodesOf< Variable >();

	auto it = std::find_if(variableNodes.begin(), variableNodes.end(), [&](const Variable* v) {
		if (v->getName() != variableName)
			return false;

		if (m_shaderGraph->findEdge(v->getInputPin(0)) == nullptr)
			return false;

		return true;
	});
	if (it == variableNodes.end())
		return;

	Variable* variable = *it;
	T_ASSERT(variable);

	m_editorGraph->deselectAllNodes();
	for (auto editorNode : m_editorGraph->getNodes())
	{
		if (editorNode->getData< Node >(L"SHADERNODE") == variable)
			editorNode->setSelected(true);
	}

	m_editorGraph->center(true);
	m_editorGraph->update();

	m_propertiesView->setPropertyObject(variable);

	event->consume();
}

void ShaderGraphEditorPage::eventUniformOrPortDoubleClick(ui::GridRowDoubleClickEvent* event)
{
	Node* node = event->getRow()->getData< Node >(L"SHADERNODE");
	if (!node)
		return;

	m_editorGraph->deselectAllNodes();
	for (auto editorNode : m_editorGraph->getNodes())
	{
		if (editorNode->getData< Node >(L"SHADERNODE") == node)
			editorNode->setSelected(true);
	}

	m_editorGraph->center(true);
	m_editorGraph->update();

	m_propertiesView->setPropertyObject(node);

	event->consume();
}

}
