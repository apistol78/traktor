/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
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

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderControl", RenderControl, ui::Widget)

RenderControl::RenderControl()
:	m_cameraHead(PI / 4.0f)
,	m_cameraPitch(-0.4f)
,	m_cameraZ(4.0f)
,	m_lastMousePosition(0, 0)
{
}

bool RenderControl::create(ui::Widget* parent, IRenderSystem* renderSystem, db::Database* database)
{
	if (!ui::Widget::create(parent, ui::WsNoCanvas))
		return false;

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 0;
	desc.multiSample = 0;
	desc.waitVBlanks = 1;
	desc.syswin = getIWidget()->getSystemWindow();

	m_renderView = renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_resourceManager = new resource::ResourceManager(database, false);
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
	m_lastMousePosition = event->getPosition();
	setCapture();
}

void RenderControl::eventButtonUp(ui::MouseButtonUpEvent* event)
{
	releaseCapture();
}

void RenderControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	if (!hasCapture())
		return;

	const ui::Point mousePosition = event->getPosition();

	const Vector2 mouseDelta(
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

	update();
}

void RenderControl::eventPaint(ui::PaintEvent* event)
{
	if (!m_renderView)
		return;

	if (!m_renderView->beginFrame())
		return;

	Clear cl;
	cl.mask = CfColor | CfDepth;
	cl.colors[0] = Color4f(0.25f, 0.25f, 0.25f, 1.0f);
	cl.depth = 1.0f;

	if (!m_renderView->beginPass(&cl, render::TfAll, render::TfAll))
		return;

	const ui::Rect innerRect = getInnerRect();
	const float aspect = float(innerRect.getSize().cx) / innerRect.getSize().cy;

	const Matrix44 viewTransform = translate(0.0f, 0.0f, m_cameraZ) * rotateX(m_cameraPitch) * rotateY(m_cameraHead);
	const Matrix44 projectionTransform = perspectiveLh(
		80.0f * PI / 180.0f,
		aspect,
		0.1f,
		2000.0f
	);

	m_primitiveRenderer->begin(0, projectionTransform);
	m_primitiveRenderer->pushView(viewTransform);

	RenderControlEvent renderEvent(this, m_renderView, m_primitiveRenderer);
	raiseEvent(&renderEvent);

	m_primitiveRenderer->popView();
	m_primitiveRenderer->end(0);
	m_primitiveRenderer->render(m_renderView, 0);

	m_renderView->endPass();
	m_renderView->endFrame();
	m_renderView->present();

	event->consume();
}

void RenderControl::eventSize(ui::SizeEvent* event)
{
	if (!m_renderView)
		return;

	const ui::Size sz = event->getSize();
	m_renderView->reset(sz.cx, sz.cy);
}

}
