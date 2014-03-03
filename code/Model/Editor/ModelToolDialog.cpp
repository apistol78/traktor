#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Model/Editor/ModelToolDialog.h"
#include "Model/Operations/CalculateConvexHull.h"
#include "Model/Operations/CleanDegenerate.h"
#include "Model/Operations/CleanDuplicates.h"
#include "Model/Operations/Quantize.h"
#include "Model/Operations/Reduce.h"
#include "Model/Operations/Transform.h"
#include "Model/Operations/Triangulate.h"
#include "Model/Operations/MergeCoplanarAdjacents.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Resource/IResourceManager.h"
#include "Ui/FileDialog.h"
#include "Ui/ListBox.h"
#include "Ui/MethodHandler.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/PaintEvent.h"
#include "Ui/Events/SizeEvent.h"
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

bool ModelToolDialog::create(ui::Widget* parent)
{
	if (!ui::Dialog::create(parent, L"Model Tool", 800, 600, ui::Dialog::WsDefaultResizable, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	Ref< ui::custom::ToolBar > toolBar = new ui::custom::ToolBar();
	toolBar->create(this);
	toolBar->addItem(new ui::custom::ToolBarButton(L"Load...", ui::Command(L"ModelTool.Load"), ui::custom::ToolBarButton::BsText));
	toolBar->addItem(new ui::custom::ToolBarButton(L"Save", ui::Command(L"ModelTool.Save"), ui::custom::ToolBarButton::BsText));
	toolBar->addItem(new ui::custom::ToolBarButton(L"Save As...", ui::Command(L"ModelTool.SaveAs"), ui::custom::ToolBarButton::BsText));
	toolBar->addItem(new ui::custom::ToolBarButton(L"Remove", ui::Command(L"ModelTool.Remove"), ui::custom::ToolBarButton::BsText));
	toolBar->addItem(new ui::custom::ToolBarSeparator());
	toolBar->addItem(new ui::custom::ToolBarButton(L"Clean Degenerate", ui::Command(L"ModelTool.CleanDegenerate"), ui::custom::ToolBarButton::BsText));
	toolBar->addItem(new ui::custom::ToolBarButton(L"Clean Duplicates", ui::Command(L"ModelTool.CleanDuplicates"), ui::custom::ToolBarButton::BsText));
	toolBar->addItem(new ui::custom::ToolBarButton(L"Quantize", ui::Command(L"ModelTool.Quantize"), ui::custom::ToolBarButton::BsText));
	toolBar->addItem(new ui::custom::ToolBarButton(L"Reduce", ui::Command(L"ModelTool.Reduce"), ui::custom::ToolBarButton::BsText));
	toolBar->addItem(new ui::custom::ToolBarButton(L"Merge Coplanar", ui::Command(L"ModelTool.MergeCoplanar"), ui::custom::ToolBarButton::BsText));
	toolBar->addItem(new ui::custom::ToolBarButton(L"Convex Hull", ui::Command(L"ModelTool.ConvexHull"), ui::custom::ToolBarButton::BsText));
	toolBar->addItem(new ui::custom::ToolBarButton(L"Triangulate", ui::Command(L"ModelTool.Triangulate"), ui::custom::ToolBarButton::BsText));
	toolBar->addItem(new ui::custom::ToolBarSeparator());

	m_toolSolid = new ui::custom::ToolBarButton(L"Solid", ui::Command(L"ModelTool.ToggleSolid"), ui::custom::ToolBarButton::BsText | ui::custom::ToolBarButton::BsToggled);
	toolBar->addItem(m_toolSolid);

	m_toolWire = new ui::custom::ToolBarButton(L"Wire", ui::Command(L"ModelTool.ToggleWire"), ui::custom::ToolBarButton::BsText | ui::custom::ToolBarButton::BsToggled);
	toolBar->addItem(m_toolWire);

	m_toolNormals = new ui::custom::ToolBarButton(L"Normals", ui::Command(L"ModelTool.ToggleNormals"), ui::custom::ToolBarButton::BsText | ui::custom::ToolBarButton::BsToggled);
	toolBar->addItem(m_toolNormals);

	m_toolVertices = new ui::custom::ToolBarButton(L"Vertices", ui::Command(L"ModelTool.ToggleVertices"), ui::custom::ToolBarButton::BsText | ui::custom::ToolBarButton::BsToggled);
	toolBar->addItem(m_toolVertices);

	toolBar->addClickEventHandler(ui::createMethodHandler(this, &ModelToolDialog::eventToolBarClick));

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(this, true, 200, false);

	m_modelList = new ui::ListBox();
	m_modelList->create(splitter, L"", ui::WsClientBorder);
	m_modelList->addSelectEventHandler(ui::createMethodHandler(this, &ModelToolDialog::eventModelListSelect));

	m_renderWidget = new ui::Widget();
	m_renderWidget->create(splitter, ui::WsClientBorder);
	m_renderWidget->addButtonDownEventHandler(ui::createMethodHandler(this, &ModelToolDialog::eventMouseDown));
	m_renderWidget->addButtonUpEventHandler(ui::createMethodHandler(this, &ModelToolDialog::eventMouseUp));
	m_renderWidget->addMouseMoveEventHandler(ui::createMethodHandler(this, &ModelToolDialog::eventMouseMove));
	m_renderWidget->addSizeEventHandler(ui::createMethodHandler(this, &ModelToolDialog::eventRenderSize));
	m_renderWidget->addPaintEventHandler(ui::createMethodHandler(this, &ModelToolDialog::eventRenderPaint));

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 0;
	desc.multiSample = 0; //m_editor->getSettings()->getProperty< PropertyInteger >(L"Editor.MultiSample", 4);
	desc.waitVBlank = false;
	desc.nativeWindowHandle = m_renderWidget->getIWidget()->getSystemHandle();

	m_renderView = m_renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(m_resourceManager, m_renderSystem))
		return false;

	m_resourceManager->bind(c_textureDebug, m_textureDebug);

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

		m_modelList->add(i->getFileName(), model);
	}

	m_modelList->update();
	return true;
}

bool ModelToolDialog::saveModel()
{
	if (!m_model)
		return false;

	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, L"Load model(s)...", L"All files;*.*", true))
		return false;

	Path fileName;
	if (fileDialog.showModal(fileName) != ui::DrOk)
	{
		fileDialog.destroy();
		return true;
	}
	fileDialog.destroy();

	return ModelFormat::writeAny(fileName, m_model);
}

bool ModelToolDialog::removeModel()
{
	int32_t selected = m_modelList->getSelected();
	if (selected >= 0)
	{
		m_modelList->remove(selected);
		m_modelList->update();

		m_model = 0;
		m_modelTris = 0;
		m_modelName = L"";
		return true;
	}
	else
		return false;
}

bool ModelToolDialog::applyOperation(const IModelOperation* operation)
{
	Ref< Model > model = new Model(*m_model);
	if (operation->apply(*model))
	{
		m_modelList->add(m_modelName + L" " + type_name(operation), model);
		m_modelList->update();

		log::info << L"Operation finished; result (\"from\" > \"to\") :" << Endl;
		log::info << L"\t" << m_model->getVertexCount() << L" > " << model->getVertexCount() << L" vertex(es)" << Endl;
		log::info << L"\t" << m_model->getPolygonCount() << L" > " << model->getPolygonCount() << L" polygon(s)" << Endl;
		log::info << L"\t" << m_model->getPositionCount() << L" > " << model->getPositionCount() << L" position(s)" << Endl;

		return true;
	}
	else
		return false;
}

void ModelToolDialog::eventToolBarClick(ui::Event* event)
{
	ui::CommandEvent* cmdEvent = checked_type_cast< ui::CommandEvent*, false >(event);
	const ui::Command& cmd = cmdEvent->getCommand();

	if (cmd == L"ModelTool.Load")
		loadModel();
	else if (cmd == L"ModelTool.Save")
		saveModel();
	else if (cmd == L"ModelTool.SaveAs")
		saveModel();
	else if (cmd == L"ModelTool.Remove")
		removeModel();
	else if (cmd == L"ModelTool.CleanDegenerate")
	{
		Ref< IModelOperation > operation = new CleanDegenerate();
		applyOperation(operation);
	}
	else if (cmd == L"ModelTool.CleanDuplicates")
	{
		Ref< IModelOperation > operation = new CleanDuplicates(0.1f);
		applyOperation(operation);
	}
	else if (cmd == L"ModelTool.Quantize")
	{
		Ref< IModelOperation > operation = new Quantize(0.5f);
		applyOperation(operation);
	}
	else if (cmd == L"ModelTool.Reduce")
	{
		Ref< IModelOperation > operation = new Reduce(0.5f);
		applyOperation(operation);
	}
	else if (cmd == L"ModelTool.MergeCoplanar")
	{
		Ref< IModelOperation > operation = new MergeCoplanarAdjacents(true);
		applyOperation(operation);
	}
	else if (cmd == L"ModelTool.ConvexHull")
	{
		Ref< IModelOperation > operation = new CalculateConvexHull();
		applyOperation(operation);
	}
	else if (cmd == L"ModelTool.Triangulate")
	{
		Ref< IModelOperation > operation = new Triangulate();
		applyOperation(operation);
	}

	m_renderWidget->update();
}

void ModelToolDialog::eventModelListSelect(ui::Event* event)
{
	m_model = m_modelList->getSelectedData< model::Model >();
	if (m_model)
	{
		m_modelTris = new Model(*m_model);
		Triangulate().apply(*m_modelTris);

		Aabb3 boundingBox = m_model->getBoundingBox();
		Vector4 extent = boundingBox.getExtent();
		float maxExtent = extent[majorAxis3(extent)];
		
		m_normalScale = maxExtent / 10.0f;
		m_modelName = m_modelList->getSelectedItem();

		log::info << L"\t" << m_model->getVertexCount() << L" vertex(es)" << Endl;
		log::info << L"\t" << m_model->getPolygonCount() << L" polygon(s)" << Endl;
		log::info << L"\t" << m_model->getPositionCount() << L" position(s)" << Endl;
	}
	else
	{
		m_modelTris = 0;
		m_modelName = L"";
	}

	m_renderWidget->update();
}

void ModelToolDialog::eventMouseDown(ui::Event* event)
{
	m_lastMousePosition = checked_type_cast< ui::MouseEvent*, false >(event)->getPosition();
	m_renderWidget->setCapture();
	m_renderWidget->setFocus();
}

void ModelToolDialog::eventMouseUp(ui::Event* event)
{
	if (m_renderWidget->hasCapture())
		m_renderWidget->releaseCapture();
}

void ModelToolDialog::eventMouseMove(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent*, false >(event);

	if (!m_renderWidget->hasCapture())
		return;

	ui::Point mousePosition = mouseEvent->getPosition();

	Vector2 mouseDelta(
		float(m_lastMousePosition.x - mousePosition.x),
		float(m_lastMousePosition.y - mousePosition.y)
	);

	if (mouseEvent->getButton() != ui::MouseEvent::BtRight)
	{
		m_cameraHead += mouseDelta.x / 100.0f;
		m_cameraPitch += mouseDelta.y / 100.0f;
	}
	else
		m_cameraZ -= mouseDelta.y * 0.1f;

	m_lastMousePosition = mousePosition;

	m_renderWidget->update();
}

void ModelToolDialog::eventRenderSize(ui::Event* event)
{
	if (!m_renderView)
		return;

	ui::SizeEvent* s = checked_type_cast< ui::SizeEvent*, false >(event);
	ui::Size sz = s->getSize();

	m_renderView->reset(sz.cx, sz.cy);
	m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));
}

void ModelToolDialog::eventRenderPaint(ui::Event* event)
{
	ui::PaintEvent* paintEvent = checked_type_cast< ui::PaintEvent* >(event);
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

	if (m_primitiveRenderer->begin(m_renderView))
	{
		m_primitiveRenderer->pushProjection(projectionTransform);
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
				Vector4 lightDir = viewTransform.inverse().axisZ();	// Light direction in object space.

				const std::vector< Vertex >& vertices = m_modelTris->getVertices();
				const std::vector< Polygon >& polygons = m_modelTris->getPolygons();
				const AlignedVector< Vector4 >& positions = m_modelTris->getPositions();

				m_primitiveRenderer->pushDepthState(true, true);
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
				m_primitiveRenderer->popDepthState();
			}

			const std::vector< Vertex >& vertices = m_model->getVertices();
			const std::vector< Polygon >& polygons = m_model->getPolygons();
			const AlignedVector< Vector4 >& positions = m_model->getPositions();
			const AlignedVector< Vector4 >& normals = m_model->getNormals();

			// Render wire-frame.
			if (m_toolWire->isToggled())
			{
				m_primitiveRenderer->pushDepthState(true, false);
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

			if (m_toolNormals->isToggled())
			{
				m_primitiveRenderer->pushDepthState(true, false);
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
				m_primitiveRenderer->pushDepthState(true, false);
				for (AlignedVector< Vector4 >::const_iterator i = positions.begin(); i != positions.end(); ++i)
				{
					m_primitiveRenderer->drawSolidPoint(*i, 3.0f, Color4ub(255, 255, 0, 200));
				}
				m_primitiveRenderer->popDepthState();
			}
		}

		m_primitiveRenderer->end();
	}

	m_renderView->end();
	m_renderView->present();

	paintEvent->consume();
}

	}
}
