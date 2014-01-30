#include <limits>
#include "Scene/Editor/DebugRenderControl.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Ui/MethodHandler.h"
#include "Ui/Widget.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.DebugRenderControl", DebugRenderControl, ui::Widget)

DebugRenderControl::DebugRenderControl()
:	m_dirtySize(0, 0)
,	m_renderOffset(0.0f, 0.0f)
,	m_renderScale(4.0f)
,	m_moveMouseOrigin(0, 0)
,	m_moveRenderOffset(0.0f, 0.0f)
{
}

bool DebugRenderControl::create(ui::Widget* parent, SceneEditorContext* context)
{
	m_context = context;
	T_ASSERT (m_context);

	m_renderWidget = new ui::Widget();
	if (!m_renderWidget->create(parent))
		return false;

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 0;
	desc.stencilBits = 0;
	desc.multiSample = 0;
	desc.waitVBlank = false;
	desc.nativeWindowHandle = m_renderWidget->getIWidget()->getSystemHandle();

	m_renderView = m_context->getRenderSystem()->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(
		m_context->getResourceManager(),
		m_context->getRenderSystem()
	))
		return false;

	m_renderWidget->addButtonDownEventHandler(ui::createMethodHandler(this, &DebugRenderControl::eventButtonDown));
	m_renderWidget->addButtonUpEventHandler(ui::createMethodHandler(this, &DebugRenderControl::eventButtonUp));
	m_renderWidget->addDoubleClickEventHandler(ui::createMethodHandler(this, &DebugRenderControl::eventDoubleClick));
	m_renderWidget->addMouseMoveEventHandler(ui::createMethodHandler(this, &DebugRenderControl::eventMouseMove));
	m_renderWidget->addMouseWheelEventHandler(ui::createMethodHandler(this, &DebugRenderControl::eventMouseWheel));
	m_renderWidget->addSizeEventHandler(ui::createMethodHandler(this, &DebugRenderControl::eventSize));
	m_renderWidget->addPaintEventHandler(ui::createMethodHandler(this, &DebugRenderControl::eventPaint));

	return true;
}

void DebugRenderControl::destroy()
{
	if (m_primitiveRenderer)
	{
		m_primitiveRenderer->destroy();
		m_primitiveRenderer = 0;
	}

	if (m_renderView)
	{
		m_renderView->close();
		m_renderView = 0;
	}

	if (m_renderWidget)
	{
		m_renderWidget->destroy();
		m_renderWidget = 0;
	}
}

void DebugRenderControl::updateWorldRenderer()
{
}

void DebugRenderControl::setAspect(float aspect)
{
}

void DebugRenderControl::setQuality(world::Quality shadowQuality, world::Quality ambientOcclusionQuality, world::Quality antiAliasQuality)
{
}

bool DebugRenderControl::handleCommand(const ui::Command& command)
{
	return false;
}

void DebugRenderControl::update()
{
	m_renderWidget->update();
}

bool DebugRenderControl::hitTest(const ui::Point& position) const
{
	return m_renderWidget->hitTest(position);
}

bool DebugRenderControl::calculateRay(const ui::Point& position, Vector4& outWorldRayOrigin, Vector4& outWorldRayDirection) const
{
	return false;
}

bool DebugRenderControl::calculateFrustum(const ui::Rect& rc, Frustum& outWorldFrustum) const
{
	return false;
}

void DebugRenderControl::moveCamera(MoveCameraMode mode, const Vector4& mouseDelta, const Vector4& viewDelta)
{
}

void DebugRenderControl::showSelectionRectangle(const ui::Rect& rect)
{
}

void DebugRenderControl::eventButtonDown(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);
	m_renderWidget->setCapture();
	m_moveMouseOrigin = mouseEvent->getPosition();
	m_moveRenderOffset = m_renderOffset;
}

void DebugRenderControl::eventButtonUp(ui::Event* event)
{
	m_renderWidget->releaseCapture();
}

void DebugRenderControl::eventDoubleClick(ui::Event* event)
{
	m_renderOffset = Vector2(0.0f, 0.0f);
	m_renderScale = 4.0f;
	m_moveMouseOrigin = ui::Point(0, 0);
	m_moveRenderOffset = Vector2(0.0f, 0.0f);

	m_renderWidget->update();
}

void DebugRenderControl::eventMouseMove(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);

	if (!m_renderWidget->hasCapture())
		return;

	ui::Size moveDelta = mouseEvent->getPosition() - m_moveMouseOrigin;
	m_renderOffset = m_moveRenderOffset + Vector2(moveDelta.cx / 100.0f, -moveDelta.cy / 100.0f);

	m_renderWidget->update();
}

void DebugRenderControl::eventMouseWheel(ui::Event* event)
{
	int32_t rotation = static_cast< ui::MouseEvent* >(event)->getWheelRotation();

	m_renderScale += rotation * 1.0f;
	m_renderScale = clamp(m_renderScale, 0.1f, 100.0f);

	m_renderWidget->update();
}

void DebugRenderControl::eventSize(ui::Event* event)
{
	if (!m_renderView || !m_renderWidget->isVisible(true))
		return;

	ui::SizeEvent* s = static_cast< ui::SizeEvent* >(event);
	ui::Size sz = s->getSize();

	if (sz.cx == m_dirtySize.cx && sz.cy == m_dirtySize.cy)
		return;

	m_renderView->reset(sz.cx, sz.cy);
	m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));

	m_dirtySize = sz;
}

void DebugRenderControl::eventPaint(ui::Event* event)
{
	if (!m_renderView || !m_primitiveRenderer)
		return;

	if (m_renderView->begin(render::EtCyclop))
	{
		const Color4f clearColor(0.7f, 0.7f, 0.7f, 0.0f);
		m_renderView->clear(
			render::CfColor | render::CfDepth,
			&clearColor,
			1.0f,
			128
		);

		const RefArray< render::ITexture >& textures = m_context->getDebugTextures();
		if (!textures.empty())
		{
			int32_t size = int32_t(std::sqrt(float(textures.size())) + 0.5f);
			float ratio = float(m_dirtySize.cx) / m_dirtySize.cy;

			Matrix44 projection;
			if (ratio < 1.0f)
			{
				projection = orthoLh(
					m_renderScale,
					m_renderScale / ratio,
					-1.0f,
					1.0f
				);
			}
			else
			{
				projection = orthoLh(
					m_renderScale * ratio,
					m_renderScale,
					-1.0f,
					1.0f
				);
			}

			m_primitiveRenderer->begin(m_renderView);
			m_primitiveRenderer->setClipDistance(100.0f);
			m_primitiveRenderer->pushProjection(projection);
			m_primitiveRenderer->pushView(Matrix44::identity());
			m_primitiveRenderer->pushDepthState(false, false);

			for (uint32_t i = 0; i < textures.size(); ++i)
			{
				float ox =  float(i % size) * 2.1f;
				float oy = -float(i / size) * 2.1f;

				ox += m_renderOffset.x;
				oy += m_renderOffset.y;

				m_primitiveRenderer->drawTextureQuad(
					Vector4(-1.0f + ox,  1.0f + oy, 0.0f, 1.0f), Vector2(0.0f, 0.0f),
					Vector4( 1.0f + ox,  1.0f + oy, 0.0f, 1.0f), Vector2(1.0f, 0.0f),
					Vector4( 1.0f + ox, -1.0f + oy, 0.0f, 1.0f), Vector2(1.0f, 1.0f),
					Vector4(-1.0f + ox, -1.0f + oy, 0.0f, 1.0f), Vector2(0.0f, 1.0f),
					Color4ub(255, 255, 255),
					textures[i]
				);
			}

			m_primitiveRenderer->end();
		}

		m_renderView->end();
		m_renderView->present();
	}

	event->consume();
}

	}
}
