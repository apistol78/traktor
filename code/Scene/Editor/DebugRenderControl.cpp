#include <limits>
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Scene/Editor/DebugRenderControl.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Ui/Widget.h"
#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const resource::Id< render::Shader > c_shader(Guid(L"{949B3C96-0196-F24E-B36E-98DD504BCE9D}"));

const wchar_t* c_visualizeTechniques[] =
{
	L"Default",
	L"Depth",
	L"Normals",
	L"Velocity",
	L"SpecularRoughness",
	L"SpecularTerm",
	L"Metalness",
	L"Reflectivity",
	L"ShadowMap",
	L"ShadowMask"
};

T_FORCE_INLINE bool DebugTargetPredicate(const render::DebugTarget& lh, const render::DebugTarget& rh)
{
	return compareIgnoreCase< std::wstring >(lh.name, rh.name) < 0;
};

		}

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
	desc.waitVBlanks = 0;
	desc.syswin = m_renderWidget->getIWidget()->getSystemWindow();

	m_renderView = m_context->getRenderSystem()->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(
		m_context->getRenderSystem()
	))
		return false;

	if (!m_context->getResourceManager()->bind(c_shader, m_shader))
		return false;

	m_renderWidget->addEventHandler< ui::MouseButtonDownEvent >(this, &DebugRenderControl::eventButtonDown);
	m_renderWidget->addEventHandler< ui::MouseButtonUpEvent >(this, &DebugRenderControl::eventButtonUp);
	m_renderWidget->addEventHandler< ui::MouseDoubleClickEvent >(this, &DebugRenderControl::eventDoubleClick);
	m_renderWidget->addEventHandler< ui::MouseMoveEvent >(this, &DebugRenderControl::eventMouseMove);
	m_renderWidget->addEventHandler< ui::MouseWheelEvent >(this, &DebugRenderControl::eventMouseWheel);
	m_renderWidget->addEventHandler< ui::SizeEvent >(this, &DebugRenderControl::eventSize);
	m_renderWidget->addEventHandler< ui::PaintEvent >(this, &DebugRenderControl::eventPaint);

	return true;
}

void DebugRenderControl::destroy()
{
	safeDestroy(m_screenRenderer);
	safeClose(m_renderView);
	safeDestroy(m_renderWidget);
}

void DebugRenderControl::updateWorldRenderer()
{
}

void DebugRenderControl::setAspect(float aspect)
{
}

void DebugRenderControl::setQuality(world::Quality imageProcessQuality, world::Quality shadowQuality, world::Quality reflectionsQuality, world::Quality motionBlurQuality, world::Quality ambientOcclusionQuality, world::Quality antiAliasQuality)
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

void DebugRenderControl::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	m_renderWidget->setCapture();
	m_moveMouseOrigin = event->getPosition();
	m_moveRenderOffset = m_renderOffset;
}

void DebugRenderControl::eventButtonUp(ui::MouseButtonUpEvent* event)
{
	m_renderWidget->releaseCapture();
}

void DebugRenderControl::eventDoubleClick(ui::MouseDoubleClickEvent* event)
{
	m_renderOffset = Vector2(0.0f, 0.0f);
	m_renderScale = 4.0f;
	m_moveMouseOrigin = ui::Point(0, 0);
	m_moveRenderOffset = Vector2(0.0f, 0.0f);
	m_renderWidget->update();
}

void DebugRenderControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	if (!m_renderWidget->hasCapture())
		return;

	ui::Size moveDelta = event->getPosition() - m_moveMouseOrigin;
	m_renderOffset = m_moveRenderOffset + Vector2(moveDelta.cx / 800.0f, -moveDelta.cy / 800.0f) * m_renderScale;

	m_renderWidget->update();
}

void DebugRenderControl::eventMouseWheel(ui::MouseWheelEvent* event)
{
	int32_t rotation = event->getRotation();

	m_renderScale += rotation * 1.0f;
	m_renderScale = clamp(m_renderScale, 0.1f, 100.0f);

	m_renderWidget->update();
}

void DebugRenderControl::eventSize(ui::SizeEvent* event)
{
	if (!m_renderView || !m_renderWidget->isVisible(true))
		return;

	ui::Size sz = event->getSize();
	if (sz.cx == m_dirtySize.cx && sz.cy == m_dirtySize.cy)
		return;

	m_renderView->reset(sz.cx, sz.cy);
	m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));

	m_dirtySize = sz;
}

void DebugRenderControl::eventPaint(ui::PaintEvent* event)
{
	if (!m_renderView || !m_screenRenderer)
		return;

	std::vector< render::DebugTarget > debugTargets = m_context->getDebugTargets();
	std::sort(debugTargets.begin(), debugTargets.end(), DebugTargetPredicate);

	if (m_renderView->begin(render::EtCyclop))
	{
		const Color4f clearColor(0.7f, 0.7f, 0.7f, 0.0f);
		m_renderView->clear(
			render::CfColor | render::CfDepth,
			&clearColor,
			1.0f,
			128
		);

		if (!debugTargets.empty())
		{
			int32_t size = int32_t(std::sqrt(float(debugTargets.size())) + 0.5f);

			for (uint32_t i = 0; i < debugTargets.size(); ++i)
			{
				float ox =  float(i % size) * 2.2f;
				float oy = -float(i / size) * 2.2f;

				ox += m_renderOffset.x;
				oy += m_renderOffset.y;

				m_shader->setTechnique(c_visualizeTechniques[debugTargets[i].visualize]);
				m_shader->setTextureParameter(L"DebugTexture", debugTargets[i].texture);
				m_shader->setVectorParameter(L"Transform", Vector4(ox, oy, m_renderScale, 0.0f));

				m_screenRenderer->draw(m_renderView, m_shader);
			}
		}

		m_renderView->end();
		m_renderView->present();
	}

	if (!debugTargets.empty())
	{
		ui::Size innerSize = m_renderWidget->getInnerRect().getSize();
		float aspect = float(innerSize.cx) / innerSize.cy;

		ui::Canvas& canvas = event->getCanvas();
		canvas.setForeground(Color4ub(0, 0, 0, 255));

		int32_t size = int32_t(std::sqrt(float(debugTargets.size())) + 0.5f);
		for (uint32_t i = 0; i < debugTargets.size(); ++i)
		{
			float ox =  float(i % size) * 2.2f;
			float oy = -float(i / size) * 2.2f;

			ox += m_renderOffset.x;
			oy += m_renderOffset.y;

			ox -= 1.0f;
			oy += 1.0f;

			oy *= aspect;

			ox *= 2.0f / m_renderScale;
			oy *= 2.0f / m_renderScale;

			ui::Size ext = canvas.getTextExtent(debugTargets[i].name);

			int32_t x = innerSize.cx * (ox * 0.5f + 0.5f);
			int32_t y = innerSize.cy * (0.5f - oy * 0.5f) - ext.cy;

			canvas.drawText(ui::Point(x, y), debugTargets[i].name);
		}
	}

	event->consume();
}

	}
}
