/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/IRuntimeClass.h"
#include "Core/Containers/BitVector.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Model/Model.h"
#include "Model/ModelAdjacency.h"
#include "Model/ModelFormat.h"
#include "Model/Pose.h"
#include "Model/Editor/ModelToolDialog.h"
#include "Model/Operations/CalculateConvexHull.h"
#include "Model/Operations/CalculateTangents.h"
#include "Model/Operations/CleanDegenerate.h"
#include "Model/Operations/CleanDuplicates.h"
#include "Model/Operations/Clear.h"
#include "Model/Operations/CullDistantFaces.h"
#include "Model/Operations/ExecuteScript.h"
#include "Model/Operations/FlattenDoubleSided.h"
#include "Model/Operations/Quantize.h"
#include "Model/Operations/Reduce.h"
#include "Model/Operations/ScaleAlongNormal.h"
#include "Model/Operations/Transform.h"
#include "Model/Operations/Triangulate.h"
#include "Model/Operations/MergeCoplanarAdjacents.h"
#include "Model/Operations/MergeTVertices.h"
#include "Model/Operations/SortCacheCoherency.h"
#include "Model/Operations/SortProjectedArea.h"
#include "Model/Operations/Unweld.h"
#include "Model/Operations/UnwrapUV.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/ITexture.h"
#include "Render/PrimitiveRenderer.h"
#include "Resource/IResourceManager.h"
#include "Script/Editor/Script.h"
#include "Ui/Application.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/TableLayout.h"
#include "Ui/FileDialog.h"
#include "Ui/FloodLayout.h"
#include "Ui/Splitter.h"
#include "Ui/StyleBitmap.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridView.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarDropDown.h"
#include "Ui/ToolBar/ToolBarSeparator.h"
#include "Ui/TreeView/TreeView.h"
#include "Ui/TreeView/TreeViewItem.h"
#include "Ui/Itf/IWidget.h"

namespace traktor::model
{
	namespace
	{

const resource::Id< render::ITexture > c_textureDebug(Guid(L"{0163BEDD-9297-A64F-AAD5-360E27E37C6E}"));

void updateSkeletonTree(Model* model, ui::TreeView* treeView, ui::TreeViewItem* parentItem, uint32_t parentNodeIndex)
{
	int32_t jointCount = model->getJointCount();
	for (int32_t i = 0; i < jointCount; ++i)
	{
		const Joint& joint = model->getJoint(i);
		if (joint.getParent() == parentNodeIndex)
		{
			int32_t affecting = 0;
			for (const auto vtx : model->getVertices())
			{
				if (vtx.getJointInfluence(i) > FUZZY_EPSILON)
					++affecting;
			}

			Ref< ui::TreeViewItem > itemJoint = treeView->createItem(
				parentItem,
				joint.getName() + L" (" + toString(affecting) + L")",
				1
			);
			itemJoint->setImage(0, 1);
			itemJoint->setData(L"JOINT", new PropertyInteger(i));

			updateSkeletonTree(model, treeView, itemJoint, i);
		}
	}
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.ModelToolDialog", ModelToolDialog, ui::Dialog)

ModelToolDialog::ModelToolDialog(
	editor::IEditor* editor,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem
)
:	m_editor(editor)
,	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
,	m_cameraHead(0.0f)
,	m_cameraPitch(0.0f)
,	m_cameraZ(10.0f)
,	m_normalScale(1.0f)
,	m_lastMousePosition(0, 0)
{
}

bool ModelToolDialog::create(ui::Widget* parent, const std::wstring& fileName, float scale)
{
	if (!ui::Dialog::create(
		parent,
		i18n::Text(L"MODEL_TOOL_TITLE"),
		1000_ut,
		800_ut,
		ui::Dialog::WsCenterParent | ui::Dialog::WsDefaultResizable,
		new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut)
	))
		return false;

	setIcon(new ui::StyleBitmap(L"Editor.Icon"));
	addEventHandler< ui::CloseEvent >(this, &ModelToolDialog::eventDialogClose);

	Ref< ui::ToolBar > toolBar = new ui::ToolBar();
	toolBar->create(this);
	toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"MODEL_TOOL_LOAD"), ui::Command(L"ModelTool.Load"), ui::ToolBarButton::BsText));
	toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"MODEL_TOOL_LOAD_TEXTURE"), ui::Command(L"ModelTool.LoadTexture"), ui::ToolBarButton::BsText));
	toolBar->addItem(new ui::ToolBarSeparator());

	m_toolShading = new ui::ToolBarButton(i18n::Text(L"MODEL_TOOL_SHADING"), ui::Command(L"ModelTool.ToggleShading"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggled);
	toolBar->addItem(m_toolShading);

	m_toolSolid = new ui::ToolBarButton(i18n::Text(L"MODEL_TOOL_SOLID"), ui::Command(L"ModelTool.ToggleSolid"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggled);
	toolBar->addItem(m_toolSolid);

	m_toolWire = new ui::ToolBarButton(i18n::Text(L"MODEL_TOOL_WIRE"), ui::Command(L"ModelTool.ToggleWire"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggle);
	toolBar->addItem(m_toolWire);

	m_toolNormals = new ui::ToolBarButton(i18n::Text(L"MODEL_TOOL_NORMALS"), ui::Command(L"ModelTool.ToggleNormals"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggle);
	toolBar->addItem(m_toolNormals);

	m_toolVertices = new ui::ToolBarButton(i18n::Text(L"MODEL_TOOL_VERTICES"), ui::Command(L"ModelTool.ToggleVertices"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggle);
	toolBar->addItem(m_toolVertices);

	m_toolCull = new ui::ToolBarButton(i18n::Text(L"MODEL_TOOL_CULL_BACKFACES"), ui::Command(L"ModelTool.ToggleCullBackfaces"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggled);
	toolBar->addItem(m_toolCull);

	m_toolNonSharedEdges = new ui::ToolBarButton(i18n::Text(L"MODEL_TOOL_NON_SHARED_EDGES"), ui::Command(L"ModelTool.ToggleNonSharedEdges"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggle);
	toolBar->addItem(m_toolNonSharedEdges);

	m_toolUV = new ui::ToolBarButton(i18n::Text(L"MODEL_TOOL_TEXCOORDS"), ui::Command(L"ModelTool.ToggleUV"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggle);
	toolBar->addItem(m_toolUV);

	m_toolChannel = new ui::ToolBarDropDown(ui::Command(L"ModelTool.Channel"), 100_ut, i18n::Text(L"MODEL_TOOL_TEXCOORD_CHANNELS"));
	toolBar->addItem(m_toolChannel);

	m_toolWeight = new ui::ToolBarButton(i18n::Text(L"MODEL_TOOL_WEIGHTS"), ui::Command(L"ModelTool.ToggleWeights"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggle);
	toolBar->addItem(m_toolWeight);

	m_toolPose = new ui::ToolBarButton(i18n::Text(L"MODEL_TOOL_POSE"), ui::Command(L"ModelTool.TogglePose"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggle);
	toolBar->addItem(m_toolPose);

	m_toolRest = new ui::ToolBarButton(i18n::Text(L"MODEL_TOOL_REST"), ui::Command(L"ModelTool.ToggleRest"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggle);
	toolBar->addItem(m_toolRest);

	toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &ModelToolDialog::eventToolBarClick);

	Ref< ui::Splitter > splitter = new ui::Splitter();
	splitter->create(this, true, 300_ut, false);

	Ref< ui::Splitter > splitterH = new ui::Splitter();
	splitterH->create(splitter, false, 30_ut, true);

	m_modelTree = new ui::TreeView();
	m_modelTree->create(splitterH, ui::WsDoubleBuffer);
	m_modelTree->addEventHandler< ui::MouseButtonDownEvent >(this, &ModelToolDialog::eventModelTreeButtonDown);
	m_modelTree->addEventHandler< ui::SelectionChangeEvent >(this, &ModelToolDialog::eventModelTreeSelect);

	Ref< ui::Tab > tab = new ui::Tab();
	tab->create(splitterH, ui::WsDoubleBuffer);

	// Material tab.
	Ref< ui::TabPage > tabPageMaterial = new ui::TabPage();
	tabPageMaterial->create(tab, i18n::Text(L"MODEL_TOOL_MATERIALS"), new ui::FloodLayout());
	tab->addPage(tabPageMaterial);

	m_materialGrid = new ui::GridView();
	m_materialGrid->create(tabPageMaterial, ui::WsDoubleBuffer | ui::GridView::WsColumnHeader);
	m_materialGrid->addColumn(new ui::GridColumn(i18n::Text(L"MODEL_TOOL_MATERIAL_NAME"), 100_ut));
	m_materialGrid->addColumn(new ui::GridColumn(i18n::Text(L"MODEL_TOOL_MATERIAL_DIFFUSE_MAP"), 100_ut));
	m_materialGrid->addColumn(new ui::GridColumn(i18n::Text(L"MODEL_TOOL_MATERIAL_SPECULAR_MAP"), 100_ut));
	m_materialGrid->addColumn(new ui::GridColumn(i18n::Text(L"MODEL_TOOL_MATERIAL_ROUGHNESS_MAP"), 100_ut));
	m_materialGrid->addColumn(new ui::GridColumn(i18n::Text(L"MODEL_TOOL_MATERIAL_METALNESS_MAP"), 100_ut));
	m_materialGrid->addColumn(new ui::GridColumn(i18n::Text(L"MODEL_TOOL_MATERIAL_TRANSPARENCY_MAP"), 100_ut));
	m_materialGrid->addColumn(new ui::GridColumn(i18n::Text(L"MODEL_TOOL_MATERIAL_EMISSIVE_MAP"), 100_ut));
	m_materialGrid->addColumn(new ui::GridColumn(i18n::Text(L"MODEL_TOOL_MATERIAL_REFLECTIVE_MAP"), 100_ut));
	m_materialGrid->addColumn(new ui::GridColumn(i18n::Text(L"MODEL_TOOL_MATERIAL_NORMAL_MAP"), 100_ut));
	m_materialGrid->addColumn(new ui::GridColumn(i18n::Text(L"MODEL_TOOL_MATERIAL_COLOR"), 110_ut));
	m_materialGrid->addColumn(new ui::GridColumn(i18n::Text(L"MODEL_TOOL_MATERIAL_DIFFUSE_TERM"), 100_ut));
	m_materialGrid->addColumn(new ui::GridColumn(i18n::Text(L"MODEL_TOOL_MATERIAL_SPECULAR_TERM"), 100_ut));
	m_materialGrid->addColumn(new ui::GridColumn(i18n::Text(L"MODEL_TOOL_MATERIAL_ROUGHNESS"), 100_ut));
	m_materialGrid->addColumn(new ui::GridColumn(i18n::Text(L"MODEL_TOOL_MATERIAL_METALNESS"), 100_ut));
	m_materialGrid->addColumn(new ui::GridColumn(i18n::Text(L"MODEL_TOOL_MATERIAL_TRANSPARENCY"), 100_ut));
	m_materialGrid->addColumn(new ui::GridColumn(i18n::Text(L"MODEL_TOOL_MATERIAL_EMISSIVE"), 100_ut));
	m_materialGrid->addColumn(new ui::GridColumn(i18n::Text(L"MODEL_TOOL_MATERIAL_REFLECTIVE"), 100_ut));
	m_materialGrid->addColumn(new ui::GridColumn(i18n::Text(L"MODEL_TOOL_MATERIAL_BLEND_OPERATOR"), 100_ut));
	m_materialGrid->addColumn(new ui::GridColumn(i18n::Text(L"MODEL_TOOL_MATERIAL_DOUBLE_SIDED"), 100_ut));
	m_materialGrid->addEventHandler< ui::SelectionChangeEvent >(this, &ModelToolDialog::eventMaterialSelect);

	// Skeleton tab.
	Ref< ui::TabPage > tabPageSkeleton = new ui::TabPage();
	tabPageSkeleton->create(tab, i18n::Text(L"MODEL_TOOL_SKELETON"), new ui::FloodLayout());
	tab->addPage(tabPageSkeleton);

	m_skeletonTree = new ui::TreeView();
	m_skeletonTree->create(tabPageSkeleton, ui::WsDoubleBuffer);
	m_skeletonTree->addImage(new ui::StyleBitmap(L"Animation.Bones", 0));
	m_skeletonTree->addImage(new ui::StyleBitmap(L"Animation.Bones", 1));
	m_skeletonTree->addEventHandler< ui::SelectionChangeEvent >(this, &ModelToolDialog::eventSkeletonSelect);

	// Statistic tab.
	Ref< ui::TabPage > tabPageStatistics = new ui::TabPage();
	tabPageStatistics->create(tab, i18n::Text(L"MODEL_TOOL_STATISTICS"), new ui::FloodLayout());
	tab->addPage(tabPageStatistics);

	m_statisticGrid = new ui::GridView();
	m_statisticGrid->create(tabPageStatistics, ui::WsDoubleBuffer | ui::GridView::WsColumnHeader);
	m_statisticGrid->addColumn(new ui::GridColumn(i18n::Text(L"MODEL_TOOL_STATISTIC_NAME"), 120_ut));
	m_statisticGrid->addColumn(new ui::GridColumn(i18n::Text(L"MODEL_TOOL_STATISTIC_VALUE"), 400_ut));

	tab->setActivePage(tabPageMaterial);

	m_modelRootPopup = new ui::Menu();
	Ref< ui::MenuItem > modelRootPopupAdd = new ui::MenuItem(i18n::Text(L"MODEL_TOOL_ADD_OPERATION"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.Clear"), L"Clear"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.CalculateTangents"), L"Calculate Tangents"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.CleanDegenerate"), L"Clean Degenerate"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.CleanDuplicates"), L"Clean Duplicates"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.ConvexHull"), L"Convex Hull"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.CullDistantFaces"), L"Cull Distant Faces"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.FlattenDoubleSided"), L"Flatten Double Sided"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.MergeCoplanar"), L"Merge Coplanar"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.MergeTVertices"), L"Merge T-vertices"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.Quantize"), L"Quantize"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.Reduce"), L"Reduce"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.ScaleAlongNormal"), L"Scale Along Normal"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.Triangulate"), L"Triangulate"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.Unweld"), L"Unweld"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.UnwrapUV"), L"Unwrap UV"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.SortCacheCoherency"), L"Sort Cache Coherency"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.SortProjectedArea"), L"Sort Projected Area"));
	modelRootPopupAdd->add(new ui::MenuItem(L"-"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.ExecuteScript"), L"Execute script..."));
	m_modelRootPopup->add(modelRootPopupAdd);
	m_modelRootPopup->add(new ui::MenuItem(L"-"));
	m_modelRootPopup->add(new ui::MenuItem(ui::Command(L"ModelTool.Reload"), i18n::Text(L"MODEL_TOOL_RELOAD")));
	m_modelRootPopup->add(new ui::MenuItem(ui::Command(L"ModelTool.SaveAs"), i18n::Text(L"MODEL_TOOL_SAVE_AS")));
	m_modelRootPopup->add(new ui::MenuItem(ui::Command(L"ModelTool.Remove"), i18n::Text(L"MODEL_TOOL_REMOVE")));

	m_modelChildPopup = new ui::Menu();
	m_modelChildPopup->add(new ui::MenuItem(ui::Command(L"ModelTool.Refresh"), i18n::Text(L"MODEL_TOOL_REFRESH")));
	m_modelChildPopup->add(new ui::MenuItem(L"-"));
	m_modelChildPopup->add(new ui::MenuItem(ui::Command(L"ModelTool.SaveAs"), i18n::Text(L"MODEL_TOOL_SAVE_AS")));
	m_modelChildPopup->add(new ui::MenuItem(ui::Command(L"ModelTool.Remove"), i18n::Text(L"MODEL_TOOL_REMOVE")));

	m_renderWidget = new ui::Widget();
	m_renderWidget->create(splitter, ui::WsNoCanvas);
	m_renderWidget->addEventHandler< ui::MouseButtonDownEvent >(this, &ModelToolDialog::eventMouseDown);
	m_renderWidget->addEventHandler< ui::MouseButtonUpEvent >(this, &ModelToolDialog::eventMouseUp);
	m_renderWidget->addEventHandler< ui::MouseMoveEvent >(this, &ModelToolDialog::eventMouseMove);
	m_renderWidget->addEventHandler< ui::SizeEvent >(this, &ModelToolDialog::eventRenderSize);
	m_renderWidget->addEventHandler< ui::PaintEvent >(this, &ModelToolDialog::eventRenderPaint);

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 0;
	desc.multiSample = 4;
	desc.waitVBlanks = 1;
	desc.syswin = m_renderWidget->getIWidget()->getSystemWindow();

	m_renderView = m_renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(m_resourceManager, m_renderSystem, 1))
		return false;

	m_resourceManager->bind(c_textureDebug, m_textureDebug);

	if (!fileName.empty())
	{
		Ref< Model > model = ModelFormat::readAny(fileName);
		if (model)
		{
			if (std::abs(scale - 1.0f) > FUZZY_EPSILON)
				Transform(traktor::scale(scale, scale, scale)).apply(*model);

			Ref< ui::TreeViewItem > item = m_modelTree->createItem(0, fileName, 0);
			item->setData(L"MODEL", model);
			item->setData(L"PATH", new PropertyString(fileName));
			item->select();

			updateModel();
		}
		else
			log::error << L"Unable to load \"" << fileName << L"\"." << Endl;
	}

	m_timer.reset();

	update();
	show();

	return true;
}

void ModelToolDialog::destroy()
{
	safeDestroy(m_primitiveRenderer);
	safeDestroy(m_texturePreview);
	safeClose(m_renderView);

	m_textureDebug.clear();

	m_resourceManager = nullptr;
	m_renderSystem = nullptr;

	ui::Dialog::destroy();
}

bool ModelToolDialog::loadModel()
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, type_name(this), i18n::Text(L"MODEL_TOOL_LOAD_MODELS"), L"All files;*.*"))
		return false;

	std::vector< Path > fileNames;
	if (fileDialog.showModal(fileNames) != ui::DialogResult::Ok)
	{
		fileDialog.destroy();
		return true;
	}
	fileDialog.destroy();

	for (auto fileName : fileNames)
	{
		Ref< Model > model = ModelFormat::readAny(fileName);
		if (!model)
			continue;

		Ref< ui::TreeViewItem > item = m_modelTree->createItem(0, fileName.getFileName(), 0);
		item->setData(L"MODEL", model);
		item->setData(L"PATH", new PropertyString(fileName.getPathName()));
	}

	m_modelTree->update();
	return true;
}

bool ModelToolDialog::loadTexture()
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, type_name(this), L"Load texture...", L"All files;*.*"))
		return false;

	Path fileName;
	if (fileDialog.showModal(fileName) != ui::DialogResult::Ok)
	{
		fileDialog.destroy();
		return true;
	}
	fileDialog.destroy();

	Ref< drawing::Image > image = drawing::Image::load(fileName);
	if (!image)
		return false;

	image->clearAlpha(1.0f);
	image->convert(drawing::PixelFormat::getR8G8B8A8().endianSwapped());

	render::SimpleTextureCreateDesc stcd;
	stcd.width = image->getWidth();
	stcd.height = image->getHeight();
	stcd.mipCount = 1;
	stcd.format = render::TfR8G8B8A8;
	stcd.sRGB = false;
	stcd.immutable = true;
	stcd.initialData[0].data = image->getData();
	stcd.initialData[0].pitch = image->getWidth() * 4;

	m_texturePreview = m_renderSystem->createSimpleTexture(stcd, T_FILE_LINE_W);

	return (bool)(m_texturePreview != nullptr);
}

bool ModelToolDialog::reloadModel(ui::TreeViewItem* itemModel)
{
	const PropertyString* ps = itemModel->getData< PropertyString >(L"PATH");
	if (!ps)
		return false;

	Ref< Model > model = ModelFormat::readAny(PropertyString::get(ps));
	if (!model)
		return false;

	itemModel->setData(L"MODEL", model);

	updateModel();
	return true;
}

bool ModelToolDialog::saveModel(Model* model)
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, type_name(this), L"Save model as...", L"All files;*.*", L"", true))
		return false;

	Path fileName;
	if (fileDialog.showModal(fileName) != ui::DialogResult::Ok)
	{
		fileDialog.destroy();
		return true;
	}
	fileDialog.destroy();

	Model clone = *model;

	int32_t channel = m_toolChannel->getSelected();
	if (channel >= 0)
	{
		auto vertices = clone.getVertices();
		for (auto& vertex : vertices)
		{
			uint32_t tc = vertex.getTexCoord(channel);
			vertex.clearTexCoords();
			vertex.setTexCoord(0, tc);
		}
		clone.setVertices(vertices);
		CleanDuplicates(0.01f).apply(clone);
	}

	return ModelFormat::writeAny(fileName, &clone);
}

void ModelToolDialog::updateModel()
{
	RefArray< ui::TreeViewItem > items;
	m_modelTree->getItems(items, ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly);

	if (items.size() == 1)
		m_model = items[0]->getData< Model >(L"MODEL");
	else
	{
		m_model = nullptr;
		m_modelTris = nullptr;
		m_modelAdjacency = nullptr;
	}

	m_materialGrid->removeAllRows();
	m_statisticGrid->removeAllRows();
	m_toolChannel->removeAll();
	m_skeletonTree->removeAllItems();

	if (m_model)
	{
		m_modelTris = new Model(*m_model);
		Triangulate().apply(*m_modelTris);

		Aabb3 boundingBox = m_model->getBoundingBox();
		Vector4 extent = boundingBox.getExtent();
		float minExtent = extent[minorAxis3(extent)];
		m_normalScale = minExtent / 10.0f;

		const AlignedVector< Material >& materials = m_model->getMaterials();
		for (AlignedVector< Material >::const_iterator i = materials.begin(); i != materials.end(); ++i)
		{
			const wchar_t* const c_blendModes[] = { L"Decal", L"Add", L"Multiply", L"Alpha", L"AlphaTest" };
			const auto& cl = i->getColor();

			Ref< ui::GridRow > row = new ui::GridRow();
			row->add(i->getName());
			row->add(i->getDiffuseMap().name + L" [" + toString(i->getDiffuseMap().channel) + L"]");
			row->add(i->getSpecularMap().name + L" [" + toString(i->getSpecularMap().channel) + L"]");
			row->add(i->getRoughnessMap().name + L" [" + toString(i->getRoughnessMap().channel) + L"]");
			row->add(i->getMetalnessMap().name + L" [" + toString(i->getMetalnessMap().channel) + L"]");
			row->add(i->getTransparencyMap().name + L" [" + toString(i->getTransparencyMap().channel) + L"]");
			row->add(i->getEmissiveMap().name + L" [" + toString(i->getEmissiveMap().channel) + L"]");
			row->add(i->getReflectiveMap().name + L" [" + toString(i->getReflectiveMap().channel) + L"]");
			row->add(i->getNormalMap().name + L" [" + toString(i->getNormalMap().channel) + L"]");
			row->add( toString(cl.getRed()) + L", " + toString(cl.getGreen()) + L", " + toString(cl.getBlue()) + L", " + toString(cl.getAlpha()));
			row->add(toString(i->getDiffuseTerm()));
			row->add(toString(i->getSpecularTerm()));
			row->add(toString(i->getRoughness()));
			row->add(toString(i->getMetalness()));
			row->add(toString(i->getTransparency()));
			row->add(toString(i->getEmissive()));
			row->add(toString(i->getReflective()));
			row->add(c_blendModes[(int32_t)i->getBlendOperator()]);
			row->add(i->isDoubleSided() ? L"Yes" : L"No");
			m_materialGrid->addRow(row);
		}

		{
			addStatistic(L"# materials", toString(m_model->getMaterials().size()));
			addStatistic(L"# vertices", toString(m_model->getVertexCount()));
			addStatistic(L"# polygons", toString(m_model->getPolygonCount()));

			SmallMap< uint32_t, uint32_t > polSizes;
			for (const auto& pol : m_model->getPolygons())
				polSizes[pol.getVertexCount()]++;

			for (auto polSize : polSizes)
				addStatistic(L"# " + toString(polSize.first) + L"-polygons", toString(polSize.second));

			addStatistic(L"# positions", toString(m_model->getPositionCount()));
			addStatistic(L"# colors", toString(m_model->getColorCount()));
			addStatistic(L"# normals", toString(m_model->getNormalCount()));
			addStatistic(L"# texcoords", toString(m_model->getTexCoords().size()));

			addStatistic(L"# texture channels", toString(m_model->getTexCoordChannels().size()));
			for (size_t i = 0; i < m_model->getTexCoordChannels().size(); ++i)
			{
				const auto& channel = m_model->getTexCoordChannels()[i];
				addStatistic(L"  " + toString(i), channel);
			}

			addStatistic(L"# joints", toString(m_model->getJointCount()));

			addStatistic(L"# animations ", toString(m_model->getAnimationCount()));
			for (size_t i = 0; i < m_model->getAnimationCount(); ++i)
			{
				const auto animation = m_model->getAnimation(i);
				addStatistic(L"  " + toString(i), animation->getName());
			}

			addStatistic(L"# blend targets", toString(m_model->getBlendTargetCount()));
		}

		uint32_t nextChannel = 8; // m_model->getAvailableTexCoordChannel();
		if (nextChannel > 0)
		{
			for (uint32_t i = 0; i < nextChannel; ++i)
				m_toolChannel->add(toString(i));

			m_toolChannel->select(0);
			m_toolChannel->setEnable(true);
		}
		else
			m_toolChannel->setEnable(false);

		updateSkeletonTree(m_model, m_skeletonTree, nullptr, c_InvalidIndex);
	}

	m_renderWidget->update();
}

void ModelToolDialog::updateOperations(ui::TreeViewItem* itemModel)
{
	T_ASSERT(itemModel->getParent() == nullptr);

	Ref< Model > model = itemModel->getData< Model >(L"MODEL");

	for (auto child : itemModel->getChildren())
	{
		Ref< const IModelOperation > operation = child->getData< IModelOperation >(L"OPERATION");
		T_ASSERT(operation != nullptr);

		// Ensure script is up-to-date so user can iterate script without adding new operation each time.
		if (auto executeScript = dynamic_type_cast< const ExecuteScript* >(operation))
		{
			const db::Instance* scriptInstance = child->getData< db::Instance >(L"SCRIPT_INSTANCE");
			T_ASSERT(scriptInstance != nullptr);

			// Ensure script has been built as we're
			// actually loading class through resource manager. 
			m_editor->buildAsset(
				scriptInstance->getGuid(),
				false
			);
			m_editor->buildWaitUntilFinished();

			// Ensure script resource isn't cached.
			m_resourceManager->reload(
				scriptInstance->getGuid(),
				false
			);

			// Load compiled script class.
			resource::Proxy< IRuntimeClass > scriptClass;
			if (!m_resourceManager->bind(
				resource::Id< IRuntimeClass >(scriptInstance->getGuid()),
				scriptClass
			))
				return;

			// Create new operation; not set in UI item though.
			operation = new ExecuteScript(scriptClass);
		}

		// Create a mutable clone of previous operation's output.
		model = DeepClone(model).create< Model >();
		T_ASSERT(model != nullptr);

		operation->apply(*model);

		child->setData(L"MODEL", model);
	}
}

void ModelToolDialog::addStatistic(const std::wstring& name, const std::wstring& value)
{
	Ref< ui::GridRow > row = new ui::GridRow();
	row->add(name);
	row->add(value);
	m_statisticGrid->addRow(row);
}

void ModelToolDialog::eventDialogClose(ui::CloseEvent* event)
{
	event->consume();
	destroy();
}

void ModelToolDialog::eventToolBarClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	if (command == L"ModelTool.Load")
		loadModel();
	else if (command == L"ModelTool.LoadTexture")
		loadTexture();

	m_renderWidget->update();
}

void ModelToolDialog::eventModelTreeButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtRight)
		return;

	RefArray< ui::TreeViewItem > items;
	if (m_modelTree->getItems(items, ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly) != 1)
		return;

	T_ASSERT(items.front());
	if (items.front()->getParent() == nullptr)
	{
		Ref< ui::TreeViewItem > itemModel = items.front();
		const ui::MenuItem* selected = m_modelRootPopup->showModal(m_modelTree, event->getPosition());
		if (selected)
		{
			const ui::Command& command = selected->getCommand();
			if (command == L"ModelTool.Clear")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Clear", 0);
				itemOperation->setData(L"OPERATION", new Clear( Model::CfMaterials | Model::CfColors | Model::CfNormals | Model::CfTexCoords | Model::CfJoints ));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.CalculateTangents")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Calculate Tangents", 0);
				itemOperation->setData(L"OPERATION", new CalculateTangents(true));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.CleanDegenerate")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Clean Degenerate", 0);
				itemOperation->setData(L"OPERATION", new CleanDegenerate());
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.CleanDuplicates")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Clean Duplicates", 0);
				itemOperation->setData(L"OPERATION", new CleanDuplicates(0.001f));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.ConvexHull")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Convex Hull", 0);
				itemOperation->setData(L"OPERATION", new CalculateConvexHull());
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.CullDistantFaces")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Cull Distant Faces", 0);
				itemOperation->setData(L"OPERATION", new CullDistantFaces(Aabb3()));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.FlattenDoubleSided")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Flatten Double Sided", 0);
				itemOperation->setData(L"OPERATION", new FlattenDoubleSided());
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.MergeCoplanar")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Merge Coplanar", 0);
				itemOperation->setData(L"OPERATION", new MergeCoplanarAdjacents());
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.MergeTVertices")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Merge T-vertices", 0);
				itemOperation->setData(L"OPERATION", new MergeTVertices());
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.Quantize")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Quantize", 0);
				itemOperation->setData(L"OPERATION", new Quantize(0.5f));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.Reduce")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Reduce", 0);
				itemOperation->setData(L"OPERATION", new Reduce(0.5f));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.ScaleAlongNormal")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Scale Along Normal", 0);
				itemOperation->setData(L"OPERATION", new ScaleAlongNormal(1.0f));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.Triangulate")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Triangulate", 0);
				itemOperation->setData(L"OPERATION", new Triangulate());
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.Unweld")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Unweld", 0);
				itemOperation->setData(L"OPERATION", new Unweld());
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.UnwrapUV")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Unwrap UV", 0);
				itemOperation->setData(L"OPERATION", new UnwrapUV(0, 1024));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.SortCacheCoherency")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Sort Cache Coherency", 0);
				itemOperation->setData(L"OPERATION", new SortCacheCoherency());
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.SortProjectedArea")
			{
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Sort Projected Area", 0);
				itemOperation->setData(L"OPERATION", new SortProjectedArea(false));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.ExecuteScript")
			{
				auto scriptInstance = m_editor->browseInstance(
					type_of< script::Script >()
				);
				if (!scriptInstance)
					return;

				// Ensure script has been built as we're
				// actually loading class through resource manager. 
				m_editor->buildAsset(
					scriptInstance->getGuid(),
					false
				);
				m_editor->buildWaitUntilFinished();

				// Ensure script resource isn't cached.
				m_resourceManager->reload(
					scriptInstance->getGuid(),
					false
				);

				// Load compiled script class.
				resource::Proxy< IRuntimeClass > scriptClass;
				if (!m_resourceManager->bind(
					resource::Id< IRuntimeClass >(scriptInstance->getGuid()),
					scriptClass
				))
					return;

				// Add operation to list.
				Ref< ui::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Execute Script", 0);
				itemOperation->setData(L"SCRIPT_INSTANCE", scriptInstance);
				itemOperation->setData(L"OPERATION", new ExecuteScript(scriptClass));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.Reload")
			{
				reloadModel(itemModel);
			}
			else if (command == L"ModelTool.SaveAs")
			{
				saveModel(itemModel->getData< Model >(L"MODEL"));
			}
			else if (command == L"ModelTool.Remove")
			{
				m_modelTree->removeItem(itemModel);
				m_model = nullptr;
				m_modelTris = nullptr;
				m_modelAdjacency = nullptr;
				m_renderWidget->update();
			}
		}
	}
	else
	{
		Ref< ui::TreeViewItem > itemOperation = items.front();
		Ref< ui::TreeViewItem > itemModel = itemOperation->getParent();
		const ui::MenuItem* selected = m_modelChildPopup->showModal(m_modelTree, event->getPosition());
		if (selected)
		{
			const ui::Command& command = selected->getCommand();
			if (command == L"ModelTool.Refresh")
			{
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.SaveAs")
			{
				saveModel(itemModel->getData< Model >(L"MODEL"));
			}
			else if (command == L"ModelTool.Remove")
			{
				m_modelTree->removeItem(itemOperation);
				updateOperations(itemModel);
				m_model = nullptr;
				m_modelTris = nullptr;
				m_modelAdjacency = nullptr;
				m_renderWidget->update();
			}
		}
	}
}

void ModelToolDialog::eventModelTreeSelect(ui::SelectionChangeEvent* event)
{
	updateModel();
}

void ModelToolDialog::eventMouseDown(ui::MouseButtonDownEvent* event)
{
	m_lastMousePosition = event->getPosition();
	m_renderWidget->setCapture();
	m_renderWidget->setFocus();
}

void ModelToolDialog::eventMouseUp(ui::MouseButtonUpEvent* event)
{
	if (m_renderWidget->hasCapture())
		m_renderWidget->releaseCapture();
}

void ModelToolDialog::eventMouseMove(ui::MouseMoveEvent* event)
{
	if (!m_renderWidget->hasCapture())
		return;

	ui::Point mousePosition = event->getPosition();

	Vector2 mouseDelta(
		float(m_lastMousePosition.x - mousePosition.x),
		float(m_lastMousePosition.y - mousePosition.y)
	);

	if (event->getButton() != ui::MbtRight)
	{
		m_cameraHead += mouseDelta.x / 100.0f;
		m_cameraPitch += mouseDelta.y / 100.0f;
	}
	else
		m_cameraZ -= mouseDelta.y * 0.1f;

	m_lastMousePosition = mousePosition;

	m_renderWidget->update();
}

void ModelToolDialog::eventRenderSize(ui::SizeEvent* event)
{
	if (!m_renderView)
		return;

	ui::Size sz = event->getSize();
	m_renderView->reset(sz.cx, sz.cy);
}

void ModelToolDialog::eventRenderPaint(ui::PaintEvent* event)
{
	ui::Rect rc = m_renderWidget->getInnerRect();

	T_ASSERT(m_renderView);
	T_ASSERT(m_primitiveRenderer);

	// Render view events; reset view if it has become lost.
	render::RenderEvent re;
	while (m_renderView->nextEvent(re))
	{
		if (re.type == render::RenderEventType::Lost)
			m_renderView->reset(rc.getWidth(), rc.getHeight());
	}

	if (!m_renderView->beginFrame())
		return;

	render::Clear cl;
	cl.mask = render::CfColor | render::CfDepth | render::CfStencil;
	cl.colors[0] = Color4f(46/255.0f, 56/255.0f, 92/255.0f, 1.0f);
	cl.depth = 1.0f;
	cl.stencil = 0;

	if (!m_renderView->beginPass(&cl, render::TfAll, render::TfAll))
		return;

	float aspect = float(rc.getWidth()) / rc.getHeight();

	Matrix44 viewTransform = translate(0.0f, 0.0f, m_cameraZ) * rotateX(m_cameraPitch) * rotateY(m_cameraHead);
	Matrix44 projectionTransform = perspectiveLh(
		80.0f * PI / 180.0f,
		aspect,
		0.01f,
		1000.0f
	);

	auto texture = (m_texturePreview != nullptr) ? m_texturePreview.ptr() : m_textureDebug.getResource();

	if (m_primitiveRenderer->begin(0, projectionTransform))
	{
		m_primitiveRenderer->pushView(viewTransform);

		for (int x = -10; x <= 10; ++x)
		{
			m_primitiveRenderer->drawLine(
				Vector4(float(x), 0.0f, -10.0f, 1.0f),
				Vector4(float(x), 0.0f, 10.0f, 1.0f),
				(x == 0) ? 2.0f : 0.0f,
				Color4ub(0, 0, 0, 80)
			);
			m_primitiveRenderer->drawLine(
				Vector4(-10.0f, 0.0f, float(x), 1.0f),
				Vector4(10.0f, 0.0f, float(x), 1.0f),
				(x == 0) ? 2.0f : 0.0f,
				Color4ub(0, 0, 0, 80)
			);
		}

		if (m_model)
		{
			T_ASSERT(m_modelTris);

			int32_t channel = m_toolChannel->getSelected();

			RefArray< ui::TreeViewItem > selectedItems;
			m_skeletonTree->getItems(selectedItems, ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly);

			int32_t weightJoint = -1;
			if (selectedItems.size() == 1)
				weightJoint = *selectedItems.front()->getData< PropertyInteger >(L"JOINT");

			// Get selection state of materials.
			const auto& rows = m_materialGrid->getRows();
			BitVector materialSelections(rows.size(), false);
			for (uint32_t i = 0; i < rows.size(); ++i)
			{
				if ((rows[i]->getState() & ui::GridRow::Selected) != 0)
					materialSelections.set(i);
			}

			// Render solid.
			if (m_toolSolid->isToggled())
			{
				bool cull = m_toolCull->isToggled();

				Vector4 eyePosition = viewTransform.inverse().translation().xyz1();	// Eye position in object space.
				Vector4 lightDir = viewTransform.inverse().axisZ();	// Light direction in object space.

				const AlignedVector< Vertex >& vertices = m_modelTris->getVertices();
				const AlignedVector< Polygon >& polygons = m_modelTris->getPolygons();
				const AlignedVector< Vector4 >& positions = m_modelTris->getPositions();

				m_primitiveRenderer->pushDepthState(true, true, false);
				for (const auto& polygon : polygons)
				{
					const auto& indices = polygon.getVertices();
					T_ASSERT(indices.size() == 3);

					Vector4 p[3];
					for (uint32_t i = 0; i < indices.size(); ++i)
					{
						const Vertex& vx0 = vertices[indices[i]];
						p[i] = positions[vx0.getPosition()];
					}

					Vector4 N = cross(p[0] - p[1], p[2] - p[1]).normalized();
					if (cull)
					{
						if (dot3(eyePosition - p[0], N) < 0)
							continue;
					}

					float diffuse = 1.0f;
					if (m_toolShading->isToggled())
						diffuse = abs(dot3(lightDir, N)) * 0.5f + 0.5f;

					Color4ub shading(
						int32_t(diffuse * 255),
						int32_t(diffuse * 255),
						int32_t(diffuse * 255),
						255						
					);

					if (!m_toolWeight->isToggled())
					{
						bool selected = false;
						if (polygon.getMaterial() != c_InvalidIndex)
							selected = materialSelections[polygon.getMaterial()];

						if (vertices[indices[0]].getTexCoordCount() > channel)
						{
							Color4ub color = 
								selected ?
								Color4ub(180, 180, 255, 255) :
								Color4ub(255, 255, 255, 255);

							m_primitiveRenderer->drawTextureTriangle(
								p[2], m_modelTris->getTexCoord(vertices[indices[2]].getTexCoord(channel)),
								p[1], m_modelTris->getTexCoord(vertices[indices[1]].getTexCoord(channel)),
								p[0], m_modelTris->getTexCoord(vertices[indices[0]].getTexCoord(channel)),
								color * shading,
								texture
							);
						}
						else
						{
							Color4ub color = 
								selected ?
								Color4ub(180, 180, 255, 255) :
								Color4ub(81, 105, 195, 255);

							m_primitiveRenderer->drawSolidTriangle(p[2], p[1], p[0], color * shading);
						}
					}
					else
					{
						const Color4ub c_errorWeight(0, 0, 255, 255);
						const Color4ub c_noWeight(0, 255, 0, 255);
						const Color4ub c_fullWeight(255, 0, 0, 255);

						m_primitiveRenderer->drawSolidTriangle(
							p[2], (vertices[indices[2]].getJointInfluenceCount() > 0) ? lerp(c_noWeight, c_fullWeight, vertices[indices[2]].getJointInfluence(weightJoint)) : c_errorWeight,
							p[1], (vertices[indices[1]].getJointInfluenceCount() > 0) ? lerp(c_noWeight, c_fullWeight, vertices[indices[1]].getJointInfluence(weightJoint)) : c_errorWeight,
							p[0], (vertices[indices[0]].getJointInfluenceCount() > 0) ? lerp(c_noWeight, c_fullWeight, vertices[indices[0]].getJointInfluence(weightJoint)) : c_errorWeight
						);
					}
				}
				m_primitiveRenderer->popDepthState();
			}

			const auto& vertices = m_model->getVertices();
			const auto& polygons = m_model->getPolygons();
			const auto& positions = m_model->getPositions();
			const auto& normals = m_model->getNormals();
			const auto& texCoords = m_model->getTexCoords();
			const auto& joints = m_model->getJoints();

			// Render wire-frame.
			if (m_toolWire->isToggled())
			{
				m_primitiveRenderer->pushDepthState(true, false, false);
				for (const auto& polygon : polygons)
				{
					const auto& indices = polygon.getVertices();
					for (uint32_t i = 0; i < indices.size(); ++i)
					{
						const Vertex& vx0 = vertices[indices[i]];
						const Vertex& vx1 = vertices[indices[(i + 1) % indices.size()]];
						const Vector4& p0 = positions[vx0.getPosition()];
						const Vector4& p1 = positions[vx1.getPosition()];
						m_primitiveRenderer->drawLine(p0, p1, Color4ub(255, 255, 255, 200));
					}
				}
				m_primitiveRenderer->popDepthState();
			}

			// Render non-shared edges.
			if (m_toolNonSharedEdges->isToggled())
			{
				// Lazy create adjacency information as it's pretty costly.
				if (!m_modelAdjacency)
					m_modelAdjacency = new ModelAdjacency(m_model, ModelAdjacency::Mode::ByPosition);

				m_primitiveRenderer->pushDepthState(true, false, false);
				for (uint32_t i = 0; i < (uint32_t)polygons.size(); ++i)
				{
					const auto& polygon = polygons[i];
					const auto& indices = polygon.getVertices();
					for (uint32_t j = 0; j < indices.size(); ++j)
					{
						uint32_t share = m_modelAdjacency->getSharedEdgeCount(i, j);
						if (share == 0)
						{
							const Vertex& vx0 = vertices[indices[j]];
							const Vertex& vx1 = vertices[indices[(j + 1) % indices.size()]];
							const Vector4& p0 = positions[vx0.getPosition()];
							const Vector4& p1 = positions[vx1.getPosition()];
							m_primitiveRenderer->drawLine(p0, p1, Color4ub(255, 40, 40, 200));
						}
					}
				}
				m_primitiveRenderer->popDepthState();
			}

			if (m_toolNormals->isToggled())
			{
				m_primitiveRenderer->pushDepthState(true, false, false);
				for (const auto& vertex : vertices)
				{
					if (vertex.getNormal() != c_InvalidIndex)
					{
						const Vector4& p = positions[vertex.getPosition()];
						const Vector4& n = normals[vertex.getNormal()];
						m_primitiveRenderer->drawLine(p, p + n * Scalar(m_normalScale), Color4ub(0, 0, 255, 200));
					}
					if (vertex.getTangent() != c_InvalidIndex)
					{
						const Vector4& p = positions[vertex.getPosition()];
						const Vector4& n = normals[vertex.getTangent()];
						m_primitiveRenderer->drawLine(p, p + n * Scalar(m_normalScale), Color4ub(255, 0, 0, 200));
					}
					if (vertex.getBinormal() != c_InvalidIndex)
					{
						const Vector4& p = positions[vertex.getPosition()];
						const Vector4& n = normals[vertex.getBinormal()];
						m_primitiveRenderer->drawLine(p, p + n * Scalar(m_normalScale), Color4ub(0, 255, 0, 200));
					}
				}
				m_primitiveRenderer->popDepthState();
			}

			if (m_toolVertices->isToggled())
			{
				m_primitiveRenderer->pushDepthState(true, false, false);
				for (const auto& position : positions)
				{
					m_primitiveRenderer->drawSolidPoint(position, 2.0f, Color4ub(255, 255, 0, 200));
				}
				m_primitiveRenderer->popDepthState();
			}

			const bool showRest = m_toolRest->isToggled();
			const bool showPose = m_toolPose->isToggled();
			if (showRest || showPose)
			{
				AlignedVector< uint32_t > childJointIds;

				m_primitiveRenderer->pushDepthState(false, false, false);

				if (showRest)
				{
					for (uint32_t i = 0; i < joints.size(); ++i)
					{
						const Color4ub colorRest = (i == weightJoint) ? Color4ub(80, 80, 255, 255) : Color4ub(120, 255, 120, 255);
						const float frameSize = (i == weightJoint) ? 0.5f : 0.25f;

						childJointIds.resize(0);
						m_modelTris->findChildJoints(i, childJointIds);

						auto Tjoint = m_modelTris->getJointGlobalTransform(i);

						m_primitiveRenderer->drawWireFrame(Tjoint.toMatrix44(), frameSize);

						if (!childJointIds.empty())
						{
							for (auto childId : childJointIds)
							{
								auto Tchild = m_modelTris->getJointGlobalTransform(childId);

								m_primitiveRenderer->drawLine(
									Tjoint.translation(),
									Tchild.translation(),
									2.0f,
									colorRest
								);
							}
						}
						else
							m_primitiveRenderer->drawSolidPoint(
								Tjoint.translation(),
								2.0f,
								colorRest
							);
					}
				}

				if (showPose && m_modelTris->getAnimationCount() > 0)
				{
					const Animation* anim = m_modelTris->getAnimation(0);

					int32_t frame = (int32_t)(m_timer.getElapsedTime() * 10.0f);
					frame %= anim->getKeyFrameCount();

					const Pose* pose = anim->getKeyFramePose(frame);

					for (uint32_t i = 0; i < joints.size(); ++i)
					{
						const Color4ub colorPose = (i == weightJoint) ? Color4ub(255, 255, 80, 255) : Color4ub(255, 180, 120, 255);
						const float frameSize = (i == weightJoint) ? 0.5f : 0.25f;

						childJointIds.resize(0);
						m_modelTris->findChildJoints(i, childJointIds);

						auto TjointPose = pose->getJointGlobalTransform(m_modelTris, i);

						m_primitiveRenderer->drawWireFrame(TjointPose.toMatrix44(), frameSize);

						if (!childJointIds.empty())
						{
							for (auto childId : childJointIds)
							{
								auto TchildPose = pose->getJointGlobalTransform(m_modelTris, childId);

								m_primitiveRenderer->drawLine(
									TjointPose.translation(),
									TchildPose.translation(),
									2.0f,
									colorPose
								);
							}
						}
						else
							m_primitiveRenderer->drawSolidPoint(
								TjointPose.translation(),
								2.0f,
								colorPose
							);
					}
				}

				m_primitiveRenderer->popDepthState();
			}

			if (m_toolUV->isToggled())
			{
				int32_t channel = m_toolChannel->getSelected();

				m_primitiveRenderer->setProjection(orthoLh(-2.0f, 2.0f, 2.0f, -2.0f, 0.0f, 1.0f));
				m_primitiveRenderer->pushView(Matrix44::identity());
				m_primitiveRenderer->pushDepthState(false, false, false);

				m_primitiveRenderer->drawTextureQuad(
					Vector4(0.0f, 0.0f, 0.5f, 1.0f),
					Vector2(0.0f, 0.0f),
					Vector4(1.0f, 0.0f, 0.5f, 1.0f),
					Vector2(1.0f, 0.0f),
					Vector4(1.0f, 1.0f, 0.5f, 1.0f),
					Vector2(1.0f, 1.0f),
					Vector4(0.0f, 1.0f, 0.5f, 1.0f),
					Vector2(0.0f, 1.0f),
					Color4ub(255, 255, 255, 200),
					texture
				);

				m_primitiveRenderer->drawWireQuad(
					Vector4(0.0f, 0.0f, 0.5f, 1.0f),
					Vector4(1.0f, 0.0f, 0.5f, 1.0f),
					Vector4(1.0f, 1.0f, 0.5f, 1.0f),
					Vector4(0.0f, 1.0f, 0.5f, 1.0f),
					Color4ub(255, 255, 255, 200)
				);

				for (uint32_t i = 0; i < polygons.size(); ++i)
				{
					const Polygon& polygon = polygons[i];
					const auto& indices = polygon.getVertices();

					for (uint32_t j = 0; j < indices.size(); ++j)
					{
						const Vertex& vx0 = vertices[indices[j]];
						const Vertex& vx1 = vertices[indices[(j + 1) % indices.size()]];

						if (vx0.getTexCoord(channel) != c_InvalidIndex && vx1.getTexCoord(channel) != c_InvalidIndex)
						{
							const Vector2& uv0 = texCoords[vx0.getTexCoord(channel)];
							const Vector2& uv1 = texCoords[vx1.getTexCoord(channel)];

							m_primitiveRenderer->drawLine(
								Vector4(uv0.x, uv0.y, 0.5f, 1.0f),
								Vector4(uv1.x, uv1.y, 0.5f, 1.0f),
								Color4ub(255, 255, 255, 200)
							);
						}
					}
				}

				m_primitiveRenderer->popView();
				m_primitiveRenderer->popDepthState();
			}

			{
				// Draw frame.
				const float c_arrowLength = 0.4f;
				const float c_frameSize = 0.2f;

				float w = 2.0f * aspect;
				float h = 2.0f;

				m_primitiveRenderer->setProjection(orthoLh(-w / 2.0f, -h / 2.0f, w / 2.0f, h / 2.0f, -1.0f, 1.0f));
				m_primitiveRenderer->pushWorld(Matrix44::identity());
				m_primitiveRenderer->pushView(
					translate(w / 2.0f - c_frameSize, h / 2.0f - c_frameSize, 0.0f) *
					scale(c_frameSize, c_frameSize, c_frameSize)
				);

				m_primitiveRenderer->pushDepthState(false, true, false);
				m_primitiveRenderer->drawSolidQuad(
					Vector4(-1.0f, 1.0f, 1.0f, 1.0f),
					Vector4(1.0f, 1.0f, 1.0f, 1.0f),
					Vector4(1.0f, -1.0f, 1.0f, 1.0f),
					Vector4(-1.0f, -1.0f, 1.0f, 1.0f),
					Color4ub(0, 0, 0, 32)
				);
				m_primitiveRenderer->popDepthState();

				m_primitiveRenderer->pushDepthState(true, true, false);

				m_primitiveRenderer->drawLine(
					Vector4::origo(),
					Vector4::origo() + viewTransform.axisX() * Scalar(1.0f - c_arrowLength),
					Color4ub(255, 0, 0, 255)
				);
				m_primitiveRenderer->drawArrowHead(
					Vector4::origo() + viewTransform.axisX() * Scalar(1.0f - c_arrowLength),
					Vector4::origo() + viewTransform.axisX(),
					0.8f,
					Color4ub(255, 0, 0, 255)
				);

				m_primitiveRenderer->drawLine(
					Vector4::origo(),
					Vector4::origo() + viewTransform.axisY() * Scalar(1.0f - c_arrowLength),
					Color4ub(0, 255, 0, 255)
				);
				m_primitiveRenderer->drawArrowHead(
					Vector4::origo() + viewTransform.axisY() * Scalar(1.0f - c_arrowLength),
					Vector4::origo() + viewTransform.axisY(),
					0.8f,
					Color4ub(0, 255, 0, 255)
				);

				m_primitiveRenderer->drawLine(
					Vector4::origo(),
					Vector4::origo() + viewTransform.axisZ() * Scalar(1.0f - c_arrowLength),
					Color4ub(0, 0, 255, 255)
				);
				m_primitiveRenderer->drawArrowHead(
					Vector4::origo() + viewTransform.axisZ() * Scalar(1.0f - c_arrowLength),
					Vector4::origo() + viewTransform.axisZ(),
					0.8f,
					Color4ub(0, 0, 255, 255)
				);

				m_primitiveRenderer->popWorld();
				m_primitiveRenderer->popView();
				m_primitiveRenderer->popDepthState();
			}
		}

		m_primitiveRenderer->end(0);
		m_primitiveRenderer->render(m_renderView, 0);
	}

	m_renderView->endPass();
	m_renderView->endFrame();
	m_renderView->present();

	event->consume();
}

void ModelToolDialog::eventMaterialSelect(ui::SelectionChangeEvent* event)
{
	m_renderWidget->update();
}

void ModelToolDialog::eventSkeletonSelect(ui::SelectionChangeEvent* event)
{
	m_renderWidget->update();
}

}
