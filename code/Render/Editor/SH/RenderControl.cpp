#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Render/Editor/SH/RenderControl.h"
#include "Render/Editor/SH/RenderControlEvent.h"
#include "Render/Resource/TextureFactory.h"
#include "Render/Resource/ShaderFactory.h"
#include "Resource/ResourceManager.h"
#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderControl", RenderControl, ui::Widget)

RenderControl::RenderControl(uint32_t matrixCount)
:	m_editMatrix(0)
{
	for (uint32_t i = 0; i < matrixCount; ++i)
	{
		m_angles.push_back(Vector4::zero());
		m_matrices.push_back(Matrix44::identity());
	}
}

bool RenderControl::create(ui::Widget* parent, IRenderSystem* renderSystem, db::Database* database)
{
	if (!ui::Widget::create(parent, ui::WsNone))
		return false;

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 0;
	desc.multiSample = 0;
	desc.waitVBlanks = 0;
	desc.syswin = getIWidget()->getSystemWindow();

	m_renderView = renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_resourceManager = new resource::ResourceManager(database, true);
	m_resourceManager->addFactory(new render::TextureFactory(renderSystem, 0));
	m_resourceManager->addFactory(new render::ShaderFactory(renderSystem));

	m_primitiveRenderer = new PrimitiveRenderer();
	if (!m_primitiveRenderer->create(m_resourceManager, renderSystem, 1))
		return false;

	addEventHandler< ui::MouseButtonDownEvent >(this, &RenderControl::eventButtonDown);
	addEventHandler< ui::MouseButtonUpEvent >(this, &RenderControl::eventButtonUp);
	addEventHandler< ui::MouseMoveEvent >(this, &RenderControl::eventMouseMove);
	addEventHandler< ui::PaintEvent >(this, &RenderControl::eventPaint);
	addEventHandler< ui::SizeEvent >(this, &RenderControl::eventSize);

	return true;
}

void RenderControl::destroy()
{
	safeDestroy(m_primitiveRenderer);
	safeClose(m_renderView);
	ui::Widget::destroy();
}

void RenderControl::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() == ui::MbtRight)
		m_editMatrix = (m_editMatrix + 1) % m_matrices.size();
	else
	{
		m_lastPoint = event->getPosition();
		setCapture();
	}
}

void RenderControl::eventButtonUp(ui::MouseButtonUpEvent* event)
{
	releaseCapture();
}

void RenderControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	if (!hasCapture())
		return;

	float deltaX = float(event->getPosition().x - m_lastPoint.x) * 0.02f;
	float deltaY = float(event->getPosition().y - m_lastPoint.y) * 0.02f;

	Vector4& angles = m_angles[m_editMatrix];
	if (event->getKeyState() && ui::KsControl)
		angles -= Vector4(0.0f, 0.0f, deltaX, 0.0f);
	else
		angles -= Vector4(deltaY, deltaX, 0.0f, 0.0f);

	Matrix44& matrix = m_matrices[m_editMatrix];
	matrix = rotateZ(angles.z()) * rotateX(angles.x()) * rotateY(angles.y());

	m_lastPoint = event->getPosition();
	update();
}

void RenderControl::eventPaint(ui::PaintEvent* event)
{
	if (!m_renderView)
		return;

	if (!m_renderView->begin(EtCyclop))
		return;

	Color4f clearColor(0.25f, 0.25f, 0.25f, 1.0f);
	m_renderView->clear(CfColor | CfDepth, &clearColor, 1.0f, 0);

	ui::Rect innerRect = getInnerRect();
	float ratio = float(innerRect.getSize().cx) / innerRect.getSize().cy;

	m_primitiveRenderer->begin(0, orthoLh(4.0f * ratio, 4.0f, -2.0f, 2.0f));
	//m_primitiveRenderer->pushProjection(perspectiveLh(45.0f * PI / 180.0f, ratio, 0.1f, 1000.0f));

	RenderControlEvent renderEvent(this, m_renderView, m_primitiveRenderer, m_angles, m_matrices);
	raiseEvent(&renderEvent);

	m_primitiveRenderer->end(0);
	m_primitiveRenderer->render(m_renderView, 0);

	m_renderView->end();
	m_renderView->present();

	event->consume();
}

void RenderControl::eventSize(ui::SizeEvent* event)
{
	if (!m_renderView)
		return;

	ui::Size sz = event->getSize();
	m_renderView->reset(sz.cx, sz.cy);
}

	}
}
