#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Model/Model.h"
#include "Model/ModelAdjacency.h"
#include "Model/ModelFormat.h"
#include "Model/Editor/ModelToolDialog.h"
#include "Model/Operations/BakePixelOcclusion.h"
#include "Model/Operations/CalculateConvexHull.h"
#include "Model/Operations/CalculateOccluder.h"
#include "Model/Operations/CalculateTangents.h"
#include "Model/Operations/CleanDegenerate.h"
#include "Model/Operations/CleanDuplicates.h"
#include "Model/Operations/CullDistantFaces.h"
#include "Model/Operations/FlattenDoubleSided.h"
#include "Model/Operations/Quantize.h"
#include "Model/Operations/Reduce.h"
#include "Model/Operations/ReduceSimplygon.h"
#include "Model/Operations/ScaleAlongNormal.h"
#include "Model/Operations/Transform.h"
#include "Model/Operations/Triangulate.h"
#include "Model/Operations/MergeCoplanarAdjacents.h"
#include "Model/Operations/UnwrapUV.h"
#include "Model/Operations/WeldHoles.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Resource/IResourceManager.h"
#include "Ui/Application.h"
#include "Ui/FileDialog.h"
#include "Ui/MenuItem.h"
#include "Ui/PopupMenu.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/Custom/ToolBar/ToolBarDropDown.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/TreeView/TreeView.h"
#include "Ui/Custom/TreeView/TreeViewItem.h"
#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace model
	{
		namespace
		{
	
const resource::Id< render::ITexture > c_textureDebug(Guid(L"{0163BEDD-9297-A64F-AAD5-360E27E37C6E}"));
		
		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.ModelToolDialog", ModelToolDialog, ui::Dialog)

ModelToolDialog::ModelToolDialog(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem
)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
,	m_cameraHead(0.0f)
,	m_cameraPitch(0.0f)
,	m_cameraZ(10.0f)
,	m_normalScale(1.0f)
,	m_lastMousePosition(0, 0)
{
}

bool ModelToolDialog::create(ui::Widget* parent, const std::wstring& fileName)
{
	if (!ui::Dialog::create(parent, L"Model Tool", ui::scaleBySystemDPI(1000), ui::scaleBySystemDPI(800), ui::Dialog::WsDefaultResizable, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	addEventHandler< ui::CloseEvent >(this, &ModelToolDialog::eventDialogClose);

	Ref< ui::custom::ToolBar > toolBar = new ui::custom::ToolBar();
	toolBar->create(this);
	toolBar->addItem(new ui::custom::ToolBarButton(L"Load...", ui::Command(L"ModelTool.Load"), ui::custom::ToolBarButton::BsText));
	toolBar->addItem(new ui::custom::ToolBarSeparator());

	m_toolSolid = new ui::custom::ToolBarButton(L"Solid", ui::Command(L"ModelTool.ToggleSolid"), ui::custom::ToolBarButton::BsText | ui::custom::ToolBarButton::BsToggled);
	toolBar->addItem(m_toolSolid);

	m_toolWire = new ui::custom::ToolBarButton(L"Wire", ui::Command(L"ModelTool.ToggleWire"), ui::custom::ToolBarButton::BsText | ui::custom::ToolBarButton::BsToggled);
	toolBar->addItem(m_toolWire);

	m_toolNormals = new ui::custom::ToolBarButton(L"Normals", ui::Command(L"ModelTool.ToggleNormals"), ui::custom::ToolBarButton::BsText | ui::custom::ToolBarButton::BsToggled);
	toolBar->addItem(m_toolNormals);

	m_toolVertices = new ui::custom::ToolBarButton(L"Vertices", ui::Command(L"ModelTool.ToggleVertices"), ui::custom::ToolBarButton::BsText | ui::custom::ToolBarButton::BsToggled);
	toolBar->addItem(m_toolVertices);

	m_toolCull = new ui::custom::ToolBarButton(L"Cull Backfaces", ui::Command(L"ModelTool.ToggleCullBackfaces"), ui::custom::ToolBarButton::BsText | ui::custom::ToolBarButton::BsToggled);
	toolBar->addItem(m_toolCull);

	m_toolNonSharedEdges = new ui::custom::ToolBarButton(L"Non-shared Edges", ui::Command(L"ModelTool.ToggleNonSharedEdges"), ui::custom::ToolBarButton::BsText | ui::custom::ToolBarButton::BsToggle);
	toolBar->addItem(m_toolNonSharedEdges);

	m_toolUV = new ui::custom::ToolBarButton(L"UV", ui::Command(L"ModelTool.ToggleUV"), ui::custom::ToolBarButton::BsText | ui::custom::ToolBarButton::BsToggle);
	toolBar->addItem(m_toolUV);

	m_toolWeight = new ui::custom::ToolBarButton(L"Weights", ui::Command(L"ModelTool.ToggleWeights"), ui::custom::ToolBarButton::BsText | ui::custom::ToolBarButton::BsToggle);
	toolBar->addItem(m_toolWeight);

	m_toolJoint = new ui::custom::ToolBarDropDown(ui::Command(L"ModelTool.Joint"), ui::scaleBySystemDPI(200), L"Joints");
	toolBar->addItem(m_toolJoint);

	toolBar->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &ModelToolDialog::eventToolBarClick);

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(this, true, ui::scaleBySystemDPI(300), false);

	Ref< ui::custom::Splitter > splitterH = new ui::custom::Splitter();
	splitterH->create(splitter, false, 50, true);

	m_modelTree = new ui::custom::TreeView();
	m_modelTree->create(splitterH, ui::WsDoubleBuffer);
	m_modelTree->addEventHandler< ui::MouseButtonDownEvent >(this, &ModelToolDialog::eventModelTreeButtonDown);
	m_modelTree->addEventHandler< ui::SelectionChangeEvent >(this, &ModelToolDialog::eventModelTreeSelect);

	m_materialGrid = new ui::custom::GridView();
	m_materialGrid->create(splitterH, ui::WsDoubleBuffer | ui::custom::GridView::WsColumnHeader);
	m_materialGrid->addColumn(new ui::custom::GridColumn(L"Name", ui::scaleBySystemDPI(100)));
	m_materialGrid->addColumn(new ui::custom::GridColumn(L"Diffuse Map", ui::scaleBySystemDPI(100)));
	m_materialGrid->addColumn(new ui::custom::GridColumn(L"Specular Map", ui::scaleBySystemDPI(100)));
	m_materialGrid->addColumn(new ui::custom::GridColumn(L"Transparency Map", ui::scaleBySystemDPI(100)));
	m_materialGrid->addColumn(new ui::custom::GridColumn(L"Emissive Map", ui::scaleBySystemDPI(100)));
	m_materialGrid->addColumn(new ui::custom::GridColumn(L"Reflective Map", ui::scaleBySystemDPI(100)));
	m_materialGrid->addColumn(new ui::custom::GridColumn(L"Normal Map", ui::scaleBySystemDPI(100)));
	m_materialGrid->addColumn(new ui::custom::GridColumn(L"Light Map", ui::scaleBySystemDPI(100)));
	m_materialGrid->addColumn(new ui::custom::GridColumn(L"Color", ui::scaleBySystemDPI(100)));
	m_materialGrid->addColumn(new ui::custom::GridColumn(L"Diffuse Term", ui::scaleBySystemDPI(100)));
	m_materialGrid->addColumn(new ui::custom::GridColumn(L"Specular Term", ui::scaleBySystemDPI(100)));
	m_materialGrid->addColumn(new ui::custom::GridColumn(L"Specular Roughness", ui::scaleBySystemDPI(100)));
	m_materialGrid->addColumn(new ui::custom::GridColumn(L"Metalness", ui::scaleBySystemDPI(100)));
	m_materialGrid->addColumn(new ui::custom::GridColumn(L"Transparency", ui::scaleBySystemDPI(100)));
	m_materialGrid->addColumn(new ui::custom::GridColumn(L"Emissive", ui::scaleBySystemDPI(100)));
	m_materialGrid->addColumn(new ui::custom::GridColumn(L"Reflective", ui::scaleBySystemDPI(100)));
	m_materialGrid->addColumn(new ui::custom::GridColumn(L"Rim Light Intensity", ui::scaleBySystemDPI(100)));
	m_materialGrid->addColumn(new ui::custom::GridColumn(L"Blend Operator", ui::scaleBySystemDPI(100)));
	m_materialGrid->addColumn(new ui::custom::GridColumn(L"Double Sided", ui::scaleBySystemDPI(100)));

	m_modelRootPopup = new ui::PopupMenu();
	m_modelRootPopup->create();

	Ref< ui::MenuItem > modelRootPopupAdd = new ui::MenuItem(L"Add Operation...");
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.CalculateTangents"), L"Calculate Tangents"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.CleanDegenerate"), L"Clean Degenerate"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.CleanDuplicates"), L"Clean Duplicates"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.ConvexHull"), L"Convex Hull"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.CullDistantFaces"), L"Cull Distant Faces"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.FlattenDoubleSided"), L"Flatten Double Sided"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.MergeCoplanar"), L"Merge Coplanar"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.Occluder"), L"Occluder"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.Quantize"), L"Quantize"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.Reduce"), L"Reduce"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.ScaleAlongNormal"), L"Scale Along Normal"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.Triangulate"), L"Triangulate"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.UnwrapUV"), L"Unwrap UV"));
	modelRootPopupAdd->add(new ui::MenuItem(ui::Command(L"ModelTool.WeldHoles"), L"Weld Holes"));
	m_modelRootPopup->add(modelRootPopupAdd);

	Ref< ui::MenuItem > modelRootPopupPerform = new ui::MenuItem(L"Perform Operation...");
	modelRootPopupPerform->add(new ui::MenuItem(ui::Command(L"ModelTool.BakeOcclusion"), L"Bake Occlusion..."));
	m_modelRootPopup->add(modelRootPopupPerform);

	m_modelRootPopup->add(new ui::MenuItem(L"-"));
	m_modelRootPopup->add(new ui::MenuItem(ui::Command(L"ModelTool.SaveAs"), L"Save As..."));
	m_modelRootPopup->add(new ui::MenuItem(ui::Command(L"ModelTool.Remove"), L"Remove"));

	m_modelChildPopup = new ui::PopupMenu();
	m_modelChildPopup->create();

	Ref< ui::MenuItem > modelChildPopupPerform = new ui::MenuItem(L"Perform Operation...");
	modelChildPopupPerform->add(new ui::MenuItem(ui::Command(L"ModelTool.BakeOcclusion"), L"Bake Occlusion..."));
	m_modelChildPopup->add(modelChildPopupPerform);

	m_modelChildPopup->add(new ui::MenuItem(L"-"));
	m_modelChildPopup->add(new ui::MenuItem(ui::Command(L"ModelTool.SaveAs"), L"Save As..."));
	m_modelChildPopup->add(new ui::MenuItem(ui::Command(L"ModelTool.Remove"), L"Remove"));

	m_renderWidget = new ui::Widget();
	m_renderWidget->create(splitter, ui::WsNone);
	m_renderWidget->addEventHandler< ui::MouseButtonDownEvent >(this, &ModelToolDialog::eventMouseDown);
	m_renderWidget->addEventHandler< ui::MouseButtonUpEvent >(this, &ModelToolDialog::eventMouseUp);
	m_renderWidget->addEventHandler< ui::MouseMoveEvent >(this, &ModelToolDialog::eventMouseMove);
	m_renderWidget->addEventHandler< ui::SizeEvent >(this, &ModelToolDialog::eventRenderSize);
	m_renderWidget->addEventHandler< ui::PaintEvent >(this, &ModelToolDialog::eventRenderPaint);

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 0;
	desc.multiSample = 0;
	desc.waitVBlanks = 0;
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
			Aabb3 boundingBox = model->getBoundingBox();
			Transform(translate(-boundingBox.getCenter())).apply(*model);

			Ref< ui::custom::TreeViewItem > item = m_modelTree->createItem(0, fileName);
			item->setData(L"MODEL", model);
		}
		else
			log::error << L"Unable to load \"" << fileName << L"\"." << Endl;
	}

	update();
	show();

	return true;
}

void ModelToolDialog::destroy()
{
	safeClose(m_renderView);
	ui::Dialog::destroy();
}

bool ModelToolDialog::loadModel()
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, L"Load model(s)...", L"All files;*.*"))
		return false;

	std::vector< Path > fileNames;
	if (fileDialog.showModal(fileNames) != ui::DrOk)
	{
		fileDialog.destroy();
		return true;
	}
	fileDialog.destroy();

	for (std::vector< Path >::const_iterator i = fileNames.begin(); i != fileNames.end(); ++i)
	{
		Ref< Model > model = ModelFormat::readAny(*i);
		if (!model)
			continue;

		Aabb3 boundingBox = model->getBoundingBox();
		Transform(translate(-boundingBox.getCenter())).apply(*model);

		Ref< ui::custom::TreeViewItem > item = m_modelTree->createItem(0, i->getFileName());
		item->setData(L"MODEL", model);
	}

	m_modelTree->update();
	return true;
}

bool ModelToolDialog::saveModel(Model* model)
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, L"Save model as...", L"All files;*.*", true))
		return false;

	Path fileName;
	if (fileDialog.showModal(fileName) != ui::DrOk)
	{
		fileDialog.destroy();
		return true;
	}
	fileDialog.destroy();

	return ModelFormat::writeAny(fileName, model);
}

void ModelToolDialog::bakeOcclusion(Model* model)
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, L"Save occlusion image as...", L"All files;*.*", true))
		return;

	Path fileName;
	if (fileDialog.showModal(fileName) != ui::DrOk)
	{
		fileDialog.destroy();
		return;
	}
	fileDialog.destroy();

	Ref< drawing::Image > imageOcclusion = new drawing::Image(
		drawing::PixelFormat::getA8R8G8B8(),
		1024,
		1024
	);
	Ref< IModelOperation > operation = new BakePixelOcclusion(
		imageOcclusion,
		1024,
		0.75f,
		0.05f
	);
	if (operation->apply(*model))
	{
		if (imageOcclusion->save(fileName))
			log::info << L"Occlusion.png saved successfully!" << Endl;
		else
			log::error << L"Unable to save " << fileName.getPathName() << Endl;
	}
	else
		log::error << L"Unable to bake occlusion" << Endl;
}

void ModelToolDialog::updateOperations(ui::custom::TreeViewItem* itemModel)
{
	T_ASSERT (itemModel->getParent() == 0);

	Ref< Model > model = itemModel->getData< Model >(L"MODEL");

	const RefArray< ui::custom::TreeViewItem >& children = itemModel->getChildren();
	for (RefArray< ui::custom::TreeViewItem >::const_iterator i = children.begin(); i != children.end(); ++i)
	{
		const IModelOperation* operation = (*i)->getData< IModelOperation >(L"OPERATION");
		T_ASSERT (operation != 0);

		model = new Model(*model);
		operation->apply(*model);

		(*i)->setData(L"MODEL", model);
	}
}

void ModelToolDialog::eventDialogClose(ui::CloseEvent* event)
{
	destroy();
}

void ModelToolDialog::eventToolBarClick(ui::custom::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	if (command == L"ModelTool.Load")
		loadModel();

	m_renderWidget->update();
}

void ModelToolDialog::eventModelTreeButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtRight)
		return;

	RefArray< ui::custom::TreeViewItem > items;
	if (m_modelTree->getItems(items, ui::custom::TreeView::GfDescendants | ui::custom::TreeView::GfSelectedOnly) != 1)
		return;

	T_ASSERT (items.front());
	if (items.front()->getParent() == 0)
	{
		Ref< ui::custom::TreeViewItem > itemModel = items.front();
		Ref< ui::MenuItem > selected = m_modelRootPopup->show(m_modelTree, event->getPosition());
		if (selected)
		{
			const ui::Command& command = selected->getCommand();
			if (command == L"ModelTool.CalculateTangents")
			{
				Ref< ui::custom::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Calculate Tangents");
				itemOperation->setData(L"OPERATION", new CalculateTangents());
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.CleanDegenerate")
			{
				Ref< ui::custom::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Clean Degenerate");
				itemOperation->setData(L"OPERATION", new CleanDegenerate());
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.CleanDuplicates")
			{
				Ref< ui::custom::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Clean Duplicates");
				itemOperation->setData(L"OPERATION", new CleanDuplicates(0.1f));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.ConvexHull")
			{
				Ref< ui::custom::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Convex Hull");
				itemOperation->setData(L"OPERATION", new CalculateConvexHull());
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.CullDistantFaces")
			{
				Ref< ui::custom::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Cull Distant Faces");
				itemOperation->setData(L"OPERATION", new CullDistantFaces(Aabb3()));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.FlattenDoubleSided")
			{
				Ref< ui::custom::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Flatten Double Sided");
				itemOperation->setData(L"OPERATION", new FlattenDoubleSided());
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.MergeCoplanar")
			{
				Ref< ui::custom::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Merge Coplanar");
				itemOperation->setData(L"OPERATION", new MergeCoplanarAdjacents(true));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.Occluder")
			{
				Ref< ui::custom::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Occluder");
				itemOperation->setData(L"OPERATION", new CalculateOccluder());
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.Quantize")
			{
				Ref< ui::custom::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Quantize");
				itemOperation->setData(L"OPERATION", new Quantize(0.5f));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.Reduce")
			{
				Ref< ui::custom::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Reduce");
				itemOperation->setData(L"OPERATION", new Reduce(0.5f));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.ScaleAlongNormal")
			{
				Ref< ui::custom::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Scale Along Normal");
				itemOperation->setData(L"OPERATION", new ScaleAlongNormal(1.0f));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.Triangulate")
			{
				Ref< ui::custom::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Triangulate");
				itemOperation->setData(L"OPERATION", new Triangulate());
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.UnwrapUV")
			{
				Ref< ui::custom::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Unwrap UV");
				itemOperation->setData(L"OPERATION", new UnwrapUV(0, 0.01f, 0.0f, 0.0f));
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.WeldHoles")
			{
				Ref< ui::custom::TreeViewItem > itemOperation = m_modelTree->createItem(itemModel, L"Weld Holes");
				itemOperation->setData(L"OPERATION", new WeldHoles());
				updateOperations(itemModel);
			}
			else if (command == L"ModelTool.BakeOcclusion")
			{
				bakeOcclusion(itemModel->getData< Model >(L"MODEL"));
			}
			else if (command == L"ModelTool.Save")
			{
				saveModel(itemModel->getData< Model >(L"MODEL"));
			}
			else if (command == L"ModelTool.Remove")
			{
				m_modelTree->removeItem(itemModel);
				m_model = 0;
				m_modelTris = 0;
				m_modelAdjacency = 0;
				m_renderWidget->update();
			}
		}
	}
	else
	{
		Ref< ui::custom::TreeViewItem > itemOperation = items.front();
		Ref< ui::custom::TreeViewItem > itemModel = itemOperation->getParent();
		Ref< ui::MenuItem > selected = m_modelChildPopup->show(m_modelTree, event->getPosition());
		if (selected)
		{
			const ui::Command& command = selected->getCommand();
			if (command == L"ModelTool.BakeOcclusion")
			{
				bakeOcclusion(itemOperation->getData< Model >(L"MODEL"));
			}
			else if (command == L"ModelTool.Save")
			{
				saveModel(itemModel->getData< Model >(L"MODEL"));
			}
			else if (command == L"ModelTool.Remove")
			{
				m_modelTree->removeItem(itemOperation);
				updateOperations(itemModel);
				m_model = 0;
				m_modelTris = 0;
				m_modelAdjacency = 0;
				m_renderWidget->update();
			}
		}
	}
}

void ModelToolDialog::eventModelTreeSelect(ui::SelectionChangeEvent* event)
{
	RefArray< ui::custom::TreeViewItem > items;
	m_modelTree->getItems(items, ui::custom::TreeView::GfDescendants | ui::custom::TreeView::GfSelectedOnly);

	if (items.size() == 1)
		m_model = items[0]->getData< Model >(L"MODEL");
	else
	{
		m_model = 0;
		m_modelTris = 0;
		m_modelAdjacency = 0;
	}

	m_materialGrid->removeAllRows();
	m_toolJoint->removeAll();

	if (m_model)
	{
		m_modelTris = new Model(*m_model);
		Triangulate().apply(*m_modelTris);

		m_modelAdjacency = new ModelAdjacency(m_model, ModelAdjacency::MdByVertex);

		Aabb3 boundingBox = m_model->getBoundingBox();
		Vector4 extent = boundingBox.getExtent();
		float maxExtent = extent[majorAxis3(extent)];
		
		m_normalScale = maxExtent / 10.0f;

		const std::vector< Material >& materials = m_model->getMaterials();
		for (std::vector< Material >::const_iterator i = materials.begin(); i != materials.end(); ++i)
		{
			Ref< ui::custom::GridRow > row = new ui::custom::GridRow();
			row->add(new ui::custom::GridItem(i->getName()));
			row->add(new ui::custom::GridItem(i->getDiffuseMap().name));
			row->add(new ui::custom::GridItem(i->getSpecularMap().name));
			row->add(new ui::custom::GridItem(i->getTransparencyMap().name));
			row->add(new ui::custom::GridItem(i->getEmissiveMap().name));
			row->add(new ui::custom::GridItem(i->getReflectiveMap().name));
			row->add(new ui::custom::GridItem(i->getNormalMap().name));
			row->add(new ui::custom::GridItem(i->getLightMap().name));
			row->add(new ui::custom::GridItem(L"0, 0, 0, 0"));
			row->add(new ui::custom::GridItem(toString(i->getDiffuseTerm())));
			row->add(new ui::custom::GridItem(toString(i->getSpecularTerm())));
			row->add(new ui::custom::GridItem(toString(i->getSpecularRoughness())));
			row->add(new ui::custom::GridItem(toString(i->getMetalness())));
			row->add(new ui::custom::GridItem(toString(i->getTransparency())));
			row->add(new ui::custom::GridItem(toString(i->getEmissive())));
			row->add(new ui::custom::GridItem(toString(i->getReflective())));
			row->add(new ui::custom::GridItem(toString(i->getRimLightIntensity())));
			row->add(new ui::custom::GridItem(L"Default"));
			row->add(new ui::custom::GridItem(i->isDoubleSided() ? L"Yes" : L"No"));
			m_materialGrid->addRow(row);
		}

		uint32_t jointCount = m_model->getJointCount();
		for (uint32_t i = 0; i < jointCount; ++i)
			m_toolJoint->add(m_model->getJoint(i));
	}

	m_renderWidget->update();
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
	m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));
}

void ModelToolDialog::eventRenderPaint(ui::PaintEvent* event)
{
	ui::Rect rc = m_renderWidget->getInnerRect();

	T_ASSERT (m_renderView);
	T_ASSERT (m_primitiveRenderer);

	if (!m_renderView->begin(render::EtCyclop))
		return;

	const Color4f clearColor(46/255.0f, 56/255.0f, 92/255.0f, 1.0f);
	m_renderView->clear(
		render::CfColor | render::CfDepth,
		&clearColor,
		1.0f,
		128
	);

	render::Viewport viewport = m_renderView->getViewport();
	float aspect = float(viewport.width) / viewport.height;

	Matrix44 viewTransform = translate(0.0f, 0.0f, m_cameraZ) * rotateY(m_cameraHead) * rotateX(m_cameraPitch);
	Matrix44 projectionTransform = perspectiveLh(
		80.0f * PI / 180.0f,
		aspect,
		0.1f,
		2000.0f
	);

	if (m_primitiveRenderer->begin(0, projectionTransform))
	{
		m_primitiveRenderer->pushView(viewTransform);

		for (int x = -10; x <= 10; ++x)
		{
			m_primitiveRenderer->drawLine(
				Vector4(float(x), 0.0f, -10.0f, 1.0f),
				Vector4(float(x), 0.0f, 10.0f, 1.0f),
				Color4ub(0, 0, 0, 80)
			);
			m_primitiveRenderer->drawLine(
				Vector4(-10.0f, 0.0f, float(x), 1.0f),
				Vector4(10.0f, 0.0f, float(x), 1.0f),
				Color4ub(0, 0, 0, 80)
			);
		}

		if (m_model)
		{
			T_ASSERT (m_modelTris);

			// Render solid.
			if (m_toolSolid->isToggled())
			{
				bool cull = m_toolCull->isToggled();

				Vector4 eyePosition = viewTransform.inverse().translation().xyz1();	// Eye position in object space.
				Vector4 lightDir = viewTransform.inverse().axisZ();	// Light direction in object space.

				const std::vector< Vertex >& vertices = m_modelTris->getVertices();
				const std::vector< Polygon >& polygons = m_modelTris->getPolygons();
				const AlignedVector< Vector4 >& positions = m_modelTris->getPositions();

				int32_t weightJoint = m_toolJoint->getSelected();

				m_primitiveRenderer->pushDepthState(true, true, false);
				for (std::vector< Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
				{
					const std::vector< uint32_t >& indices = i->getVertices();
					T_ASSERT (indices.size() == 3);

					Vector4 p[3];

					for (uint32_t i = 0; i < indices.size(); ++i)
					{
						const Vertex& vx0 = vertices[indices[i]];
						p[i] = positions[vx0.getPosition()];
					}

					Vector4 N = cross(p[0] - p[1], p[2] - p[1]).normalized();
					float diffuse = abs(dot3(lightDir, N)) * 0.5f + 0.5f;

					if (cull)
					{
						if (dot3(eyePosition - p[0], N) < 0)
							continue;
					}

					if (!m_toolWeight->isToggled())
					{
						if (vertices[indices[0]].getTexCoordCount() > 0)
						{
							m_primitiveRenderer->drawTextureTriangle(
								p[2], m_modelTris->getTexCoord(vertices[indices[2]].getTexCoord(0)),
								p[1], m_modelTris->getTexCoord(vertices[indices[1]].getTexCoord(0)),
								p[0], m_modelTris->getTexCoord(vertices[indices[0]].getTexCoord(0)),
								Color4ub(
									int32_t(diffuse * 255),
									int32_t(diffuse * 255),
									int32_t(diffuse * 255),
									255
								),
								m_textureDebug
							);
						}
						else
						{
							m_primitiveRenderer->drawSolidTriangle(p[2], p[1], p[0], Color4ub(
								int32_t(diffuse * 81),
								int32_t(diffuse * 105),
								int32_t(diffuse * 195),
								255
							));
						}
					}
					else
					{
						const Color4ub c_noWeight(0, 255, 0, 255);
						const Color4ub c_fullWeight(255, 0, 0, 255);

						m_primitiveRenderer->drawSolidTriangle(
							p[2], lerp(c_noWeight, c_fullWeight, vertices[indices[2]].getJointInfluence(weightJoint)),
							p[1], lerp(c_noWeight, c_fullWeight, vertices[indices[1]].getJointInfluence(weightJoint)),
							p[0], lerp(c_noWeight, c_fullWeight, vertices[indices[0]].getJointInfluence(weightJoint))
						);
					}
				}
				m_primitiveRenderer->popDepthState();
			}

			const std::vector< Vertex >& vertices = m_model->getVertices();
			const std::vector< Polygon >& polygons = m_model->getPolygons();
			const AlignedVector< Vector4 >& positions = m_model->getPositions();
			const AlignedVector< Vector4 >& normals = m_model->getNormals();
			const AlignedVector< Vector2 >& texCoords = m_model->getTexCoords();

			// Render wire-frame.
			if (m_toolWire->isToggled())
			{
				m_primitiveRenderer->pushDepthState(true, false, false);
				for (std::vector< Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
				{
					const std::vector< uint32_t >& indices = i->getVertices();

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
				m_primitiveRenderer->pushDepthState(true, false, false);
				for (uint32_t i = 0; i < polygons.size(); ++i)
				{
					const Polygon& polygon = polygons[i];
					const std::vector< uint32_t >& indices = polygon.getVertices();

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
				for (std::vector< Vertex >::const_iterator i = vertices.begin(); i != vertices.end(); ++i)
				{
					if (i->getNormal() != c_InvalidIndex)
					{
						const Vector4& p = positions[i->getPosition()];
						const Vector4& n = normals[i->getNormal()];

						m_primitiveRenderer->drawLine(p, p + n * Scalar(m_normalScale), Color4ub(0, 255, 0, 200));
					}
				}
				m_primitiveRenderer->popDepthState();
			}

			if (m_toolVertices->isToggled())
			{
				m_primitiveRenderer->pushDepthState(true, false, false);
				for (AlignedVector< Vector4 >::const_iterator i = positions.begin(); i != positions.end(); ++i)
				{
					m_primitiveRenderer->drawSolidPoint(*i, 3.0f, Color4ub(255, 255, 0, 200));
				}
				m_primitiveRenderer->popDepthState();
			}

			if (m_toolUV->isToggled())
			{
				m_primitiveRenderer->setProjection(orthoLh(-2.0f, 2.0f, 2.0f, -2.0f, 0.0f, 1.0f));
				m_primitiveRenderer->pushView(Matrix44::identity());
				m_primitiveRenderer->pushDepthState(false, false, false);

				for (uint32_t i = 0; i < polygons.size(); ++i)
				{
					const Polygon& polygon = polygons[i];
					const std::vector< uint32_t >& indices = polygon.getVertices();

					for (uint32_t j = 0; j < indices.size(); ++j)
					{
						const Vertex& vx0 = vertices[indices[j]];
						const Vertex& vx1 = vertices[indices[(j + 1) % indices.size()]];

						if (vx0.getTexCoord(0) != c_InvalidIndex && vx1.getTexCoord(0) != c_InvalidIndex)
						{
							const Vector2& uv0 = texCoords[vx0.getTexCoord(0)];
							const Vector2& uv1 = texCoords[vx1.getTexCoord(0)];

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
		}

		m_primitiveRenderer->end(0);
		m_primitiveRenderer->render(m_renderView, 0);
	}

	m_renderView->end();
	m_renderView->present();

	event->consume();
}

	}
}
