/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/RenderControl.h"

#include "Core/Math/Color4f.h"
#include "Core/Math/Vector2.h"
#include "Core/Misc/ObjectStore.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Database/Database.h"
#include "Editor/IEditor.h"
#include "Render/Editor/RenderControlEvent.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/TextureFactory.h"
#include "Resource/ResourceManager.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/StyleSheet.h"

namespace traktor::render
{
namespace
{

const float c_deltaMoveScale = 0.025f;
const float c_deltaScaleHead = 0.015f;
const float c_deltaScalePitch = 0.005f;

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderControl", RenderControl, ui::Widget)

bool RenderControl::create(ui::Widget* parent, editor::IEditor* editor)
{
	Ref< db::Database > database = editor->getOutputDatabase();
	if (!database)
		return false;

	IRenderSystem* renderSystem = editor->getObjectStore()->get< IRenderSystem >();
	if (!renderSystem)
		return false;

	Ref< resource::ResourceManager > resourceManager = new resource::ResourceManager(
		database,
		editor->getSettings()->getProperty< bool >(L"Resource.Verbose", false));
	resourceManager->addFactory(new TextureFactory(renderSystem, 0));
	resourceManager->addFactory(new ShaderFactory(renderSystem));

	m_resourceManager = resourceManager;
	m_ownResourceManager = true;

	return createInternal(parent, editor);
}

bool RenderControl::create(ui::Widget* parent, editor::IEditor* editor, resource::IResourceManager* resourceManager)
{
	m_resourceManager = resourceManager;
	m_ownResourceManager = false;

	return createInternal(parent, editor);
}

bool RenderControl::createInternal(ui::Widget* parent, editor::IEditor* editor)
{
	IRenderSystem* renderSystem = editor->getObjectStore()->get< IRenderSystem >();
	if (!renderSystem)
		return false;

	if (!ui::Widget::create(parent, ui::WsFocus | ui::WsNoCanvas))
		return false;

	const PropertyGroup* settings = editor->getSettings();

	m_renderSystem = renderSystem;
	m_multiSample = (uint32_t)settings->getProperty< int32_t >(L"Editor.MultiSample", 4);

	RenderViewEmbeddedDesc rvd;
	rvd.depthBits = 32;
	rvd.stencilBits = 0;
	rvd.multiSample = m_multiSample;
	rvd.multiSampleShading = settings->getProperty< float >(L"Editor.MultiSampleShading", 0.0f);
	rvd.allowHDR = settings->getProperty< bool >(L"Editor.HDR", true);
	rvd.waitVBlanks = 1;
	rvd.syswin = getIWidget()->getSystemWindow();

	m_renderView = renderSystem->createRenderView(rvd);
	if (!m_renderView)
		return false;

	m_primitiveRenderer = new PrimitiveRenderer();
	if (!m_primitiveRenderer->create(m_resourceManager, renderSystem, 1))
		return false;

	addEventHandler< ui::MouseButtonDownEvent >(this, &RenderControl::eventButtonDown);
	addEventHandler< ui::MouseButtonUpEvent >(this, &RenderControl::eventButtonUp);
	addEventHandler< ui::MouseMoveEvent >(this, &RenderControl::eventMouseMove);
	addEventHandler< ui::PaintEvent >(this, &RenderControl::eventPaint);

	return true;
}

void RenderControl::destroy()
{
	safeDestroy(m_primitiveRenderer);
	safeClose(m_renderView);

	if (m_ownResourceManager)
		safeDestroy(m_resourceManager);
	else
		m_resourceManager = nullptr;

	m_renderSystem = nullptr;

	ui::Widget::destroy();
}

void RenderControl::setPerspective(float fieldOfView, float nearZ, float farZ)
{
	m_fieldOfView = fieldOfView;
	m_nearZ = nearZ;
	m_farZ = farZ;
}

void RenderControl::drawGrid(PrimitiveRenderer* primitiveRenderer, float extent, float step) const
{
	const Color4ub gridColor = getGridColor();
	const Color4ub axisColor = getGridAxisColor();
	const int32_t count = (int32_t)(extent / step);

	for (int32_t i = -count; i <= count; ++i)
	{
		const float f = float(i) * step;
		const Color4ub color = (i == 0) ? axisColor : gridColor;
		primitiveRenderer->drawLine(Vector4(f, 0.0f, -extent, 1.0f), Vector4(f, 0.0f, extent, 1.0f), color);
		primitiveRenderer->drawLine(Vector4(-extent, 0.0f, f, 1.0f), Vector4(extent, 0.0f, f, 1.0f), color);
	}
}

Color4ub RenderControl::getGridColor() const
{
	return getStyleSheet()->getColor(this, L"color-grid");
}

Color4ub RenderControl::getGridAxisColor() const
{
	return getStyleSheet()->getColor(this, L"color-grid-axis");
}

void RenderControl::setView(const View& view)
{
	m_view = view;
	update();
}

Matrix44 RenderControl::getViewTransform() const
{
	return translate(m_view.position) * rotateX(m_view.pitch) * rotateY(m_view.head);
}

bool RenderControl::validateRenderView()
{
	if (!m_renderView)
		return false;

	bool lost = false;
	for (RenderEvent re = {}; m_renderView->nextEvent(re);)
		if (re.type == RenderEventType::Lost)
			lost = true;

	const ui::Size sz = getInnerRect().getSize();
	if (lost || sz.cx != m_renderSize.cx || sz.cy != m_renderSize.cy)
	{
		if (!m_renderView->reset(sz.cx, sz.cy))
			return false;
		m_renderSize = sz;
	}

	return true;
}

bool RenderControl::renderFrame()
{
	if (!validateRenderView())
		return false;

	if (!m_renderView->beginFrame())
		return false;

	Clear cl;
	cl.mask = CfColor | CfDepth;
	cl.colors[0] = Color4f::fromColor4ub(getStyleSheet()->getColor(this, L"background-color"));
	cl.depth = 1.0f;

	if (!m_renderView->beginPass(&cl, TfAll, TfAll))
	{
		m_renderView->endFrame();
		return false;
	}

	const float aspect = float(m_renderSize.cx) / m_renderSize.cy;
	const Matrix44 projectionTransform = perspectiveLh(
		m_fieldOfView,
		aspect,
		m_nearZ,
		m_farZ);

	if (m_primitiveRenderer->begin(0, projectionTransform))
	{
		m_primitiveRenderer->pushView(getViewTransform());

		RenderControlEvent renderEvent(this, m_renderView, m_primitiveRenderer);
		raiseEvent(&renderEvent);

		m_primitiveRenderer->popView();
		m_primitiveRenderer->end(0);
		m_primitiveRenderer->render(m_renderView, 0);
	}

	m_renderView->endPass();
	m_renderView->endFrame();
	m_renderView->present();

	return true;
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
		float(m_lastMousePosition.y - mousePosition.y));

	if (event->getButton() == ui::MbtLeft)
	{
		if ((event->getKeyState() & ui::KsControl) == 0)
		{
			// Move X/Z direction.
			const float dx = -mouseDelta.x * c_deltaMoveScale;
			const float dz = -mouseDelta.y * c_deltaMoveScale;
			m_view.position += Vector4(dx, 0.0f, dz, 0.0f);
		}
		else
		{
			// Move X/Y direction.
			const float dx = -mouseDelta.x * c_deltaMoveScale;
			const float dy = mouseDelta.y * c_deltaMoveScale;
			m_view.position += Vector4(dx, dy, 0.0f, 0.0f);
		}
	}
	else if (event->getButton() == ui::MbtRight)
	{
		m_view.head += mouseDelta.x * c_deltaScaleHead;
		m_view.pitch += mouseDelta.y * c_deltaScalePitch;
	}

	m_lastMousePosition = mousePosition;

	update();
}

void RenderControl::eventPaint(ui::PaintEvent* event)
{
	if (renderFrame())
		event->consume();
}

}
