#include <ctime>
#include "Spray/Editor/EffectPreviewControl.h"
#include "Spray/Editor/ConeSourceRenderer.h"
#include "Spray/Editor/DiscSourceRenderer.h"
#include "Spray/Editor/PointSourceRenderer.h"
#include "Spray/Editor/QuadSourceRenderer.h"
#include "Spray/Editor/SphereSourceRenderer.h"
#include "Spray/Editor/SplineSourceRenderer.h"
#include "Spray/PointRenderer.h"
#include "Spray/Effect.h"
#include "Spray/EffectLayer.h"
#include "Spray/EffectInstance.h"
#include "Spray/Emitter.h"
#include "Spray/Sources/ConeSource.h"
#include "Spray/Sources/DiscSource.h"
#include "Spray/Sources/PointSource.h"
#include "Spray/Sources/QuadSource.h"
#include "Spray/Sources/SphereSource.h"
#include "Spray/Sources/SplineSource.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Application.h"
#include "Ui/PopupMenu.h"
#include "Ui/MenuItem.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/Events/IdleEvent.h"
#include "Ui/Custom/ColorPicker/ColorDialog.h"
#include "I18N/Text.h"
#include "Render/RenderSystem.h"
#include "Render/RenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Render/Context/RenderContext.h"
#include "World/WorldRenderView.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace spray
	{
		namespace
		{

const uint32_t c_initialRandomSeed = 5489UL;
const int c_updateInterval = 30;
const float c_deltaScaleX = 0.015f;
const float c_deltaScaleY = 0.025f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectPreviewControl", EffectPreviewControl, ui::Widget)

EffectPreviewControl::EffectPreviewControl()
:	m_randomSeed(c_initialRandomSeed)
,	m_context(0.0f, c_initialRandomSeed)
,	m_effectPosition(0.0f, -2.0f, 7.0f, 1.0f)
,	m_angleHead(0.0f)
,	m_colorBackground(0x40, 0x42, 0x45)
,	m_timeScale(1.0f)
,	m_lastDeltaTime(1.0f / c_updateInterval)
,	m_guideVisible(true)
,	m_velocityVisible(false)
{
	m_sourceRenderers[&type_of< ConeSource >()] = gc_new< ConeSourceRenderer >();
	m_sourceRenderers[&type_of< DiscSource >()] = gc_new< DiscSourceRenderer >();
	m_sourceRenderers[&type_of< PointSource >()] = gc_new< PointSourceRenderer >();
	m_sourceRenderers[&type_of< QuadSource >()] = gc_new< QuadSourceRenderer >();
	m_sourceRenderers[&type_of< SphereSource >()] = gc_new< SphereSourceRenderer >();
	m_sourceRenderers[&type_of< SplineSource >()] = gc_new< SplineSourceRenderer >();
}

bool EffectPreviewControl::create(ui::Widget* parent, int style, render::RenderSystem* renderSystem)
{
	if (!Widget::create(parent, style))
		return false;

	render::RenderViewCreateDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 0;
	desc.multiSample = 4;
	desc.waitVBlank = false;
	desc.mipBias = 0.0f;

	m_renderView = renderSystem->createRenderView(getIWidget()->getSystemHandle(), desc);
	if (!m_renderView)
		return false;

	m_primitiveRenderer = gc_new< render::PrimitiveRenderer >();
	if (!m_primitiveRenderer->create(renderSystem))
		return false;

	m_renderContext = gc_new< render::RenderContext >(m_renderView);
	m_pointRenderer = gc_new< PointRenderer >(renderSystem, 1.0f, 1.0f);

	addButtonDownEventHandler(ui::createMethodHandler(this, &EffectPreviewControl::eventButtonDown));
	addButtonUpEventHandler(ui::createMethodHandler(this, &EffectPreviewControl::eventButtonUp));
	addMouseMoveEventHandler(ui::createMethodHandler(this, &EffectPreviewControl::eventMouseMove));
	addSizeEventHandler(ui::createMethodHandler(this, &EffectPreviewControl::eventSize));
	addPaintEventHandler(ui::createMethodHandler(this, &EffectPreviewControl::eventPaint));

	m_timer.start();

	m_menuOptions = gc_new< ui::PopupMenu >();
	m_menuOptions->create();
	m_menuOptions->add(gc_new< ui::MenuItem >(ui::Command(L"Effect.Editor.ChangeBackgroundColor"), L"Change background color..."));

	// Register our event handler in case of message idle.
	m_idleHandler = ui::createMethodHandler(this, &EffectPreviewControl::eventIdle);
	ui::Application::getInstance().addEventHandler(ui::EiIdle, m_idleHandler);

	return true;
}

void EffectPreviewControl::destroy()
{
	if (m_idleHandler)
	{
		ui::Application::getInstance().removeEventHandler(ui::EiIdle, m_idleHandler);
		m_idleHandler = 0;
	}

	if (m_menuOptions)
	{
		m_menuOptions->destroy();
		m_menuOptions = 0;
	}

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

	Widget::destroy();
}

void EffectPreviewControl::setEffect(Effect* effect)
{
	m_effect = effect;
	m_effectInstance = m_effect->createInstance();
}

void EffectPreviewControl::setTimeScale(float timeScale)
{
	m_timeScale = timeScale;
}

void EffectPreviewControl::setTotalTime(float totalTime)
{
	if (m_effectInstance)
		m_effectInstance->setTime(totalTime);
}

void EffectPreviewControl::showGuide(bool guideVisible)
{
	m_guideVisible = guideVisible;
}

void EffectPreviewControl::showVelocity(bool velocityVisible)
{
	m_velocityVisible = velocityVisible;
}

void EffectPreviewControl::randomizeSeed()
{
	m_randomSeed = clock();
	m_context.random = RandomGeometry(m_randomSeed);
}

void EffectPreviewControl::syncEffect()
{
	EmitterUpdateContext syncContext(0.0f, m_randomSeed);

	float currentTime = m_effectInstance->getTime();

	// Re-create instance.
	m_effectInstance = m_effect->createInstance();

	// Run emitter until total time is reached.
	const float c_deltaTime = 1.0f / 30.0f;
	for (float T = 0.0f; T < currentTime; T += c_deltaTime)
	{
		float deltaTime = min(c_deltaTime, currentTime - T);
		syncContext.deltaTime = deltaTime;

		m_effectInstance->update(syncContext, Matrix44::identity(), true);
		m_effectInstance->synchronize();
	}

	m_context = syncContext;
}

void EffectPreviewControl::eventButtonDown(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);
	if (mouseEvent->getButton() == ui::MouseEvent::BtLeft)
	{
		m_lastMousePosition = mouseEvent->getPosition();
		setCapture();
	}
	else if (mouseEvent->getButton() == ui::MouseEvent::BtRight)
	{
		Ref< ui::MenuItem > selected = m_menuOptions->show(this, mouseEvent->getPosition());
		if (!selected)
			return;

		if (selected->getCommand() == L"Effect.Editor.ChangeBackgroundColor")
		{
			ui::custom::ColorDialog dialogColor;
			dialogColor.create(this, i18n::Text(L"COLOR_DIALOG_TEXT"), ui::custom::ColorDialog::WsDefaultFixed, m_colorBackground);
			if (dialogColor.showModal() == ui::DrOk)
				m_colorBackground = dialogColor.getColor();
			dialogColor.destroy();
		}
	}
}

void EffectPreviewControl::eventButtonUp(ui::Event* event)
{
	releaseCapture();
}

void EffectPreviewControl::eventMouseMove(ui::Event* event)
{
	if (!hasCapture())
		return;

	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);
	
	if ((mouseEvent->getKeyState() & ui::KsControl) == ui::KsControl)
		m_effectPosition += Vector4(0.0f, float(m_lastMousePosition.y - mouseEvent->getPosition().y) * c_deltaScaleY, 0.0f, 0.0f);
	else
		m_effectPosition += Vector4(0.0f, 0.0f, -float(m_lastMousePosition.y - mouseEvent->getPosition().y) * c_deltaScaleY, 0.0f);

	m_angleHead += float(m_lastMousePosition.x - mouseEvent->getPosition().x) * c_deltaScaleX;

	m_lastMousePosition = mouseEvent->getPosition();

	update();
}

void EffectPreviewControl::eventSize(ui::Event* event)
{
	if (!m_renderView)
		return;

	ui::SizeEvent* s = static_cast< ui::SizeEvent* >(event);
	ui::Size sz = s->getSize();

	m_renderView->resize(sz.cx, sz.cy);
	m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));
}

void EffectPreviewControl::eventPaint(ui::Event* event)
{
	if (!m_renderView)
		return;

	if (!m_renderView->begin())
		return;

	const float clearColor[] =
	{
		m_colorBackground.r / 255.0f,
		m_colorBackground.g / 255.0f,
		m_colorBackground.b / 255.0f,
		0.0f
	};
	m_renderView->clear(
		render::CfColor | render::CfDepth,
		clearColor,
		1.0f,
		128
	);

	render::Viewport viewport = m_renderView->getViewport();
	float aspect = float(viewport.width) / viewport.height;

	Matrix44 viewTransform = rotateY(m_angleHead) * translate(m_effectPosition);
	Matrix44 projectionTransform = perspectiveLh(
		80.0f * PI / 180.0f,
		aspect,
		0.1f,
		2000.0f
	);

	Matrix44 viewInverse = viewTransform.inverseOrtho();
	Plane cameraPlane(
		viewInverse.axisZ(),
		viewInverse.translation()
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
				Color(90, 90, 90)
			);
			m_primitiveRenderer->drawLine(
				Vector4(-10.0f, 0.0f, float(x), 1.0f),
				Vector4(10.0f, 0.0f, float(x), 1.0f),
				Color(90, 90, 90)
			);
		}

		// Draw emitter sources.
		if (m_effect && m_guideVisible)
		{
			const RefArray< EffectLayer >& layers = m_effect->getLayers();
			for (RefArray< EffectLayer >::const_iterator i = layers.begin(); i != layers.end(); ++i)
			{
				Ref< Emitter > emitter = (*i)->getEmitter();
				if (!emitter)
					continue;

				Ref< Source > source = emitter->getSource();
				if (!source)
					continue;

				std::map< const Type*, Ref< SourceRenderer > >::const_iterator j = m_sourceRenderers.find(&source->getType());
				if (j != m_sourceRenderers.end())
					j->second->render(m_primitiveRenderer, source);
			}
		}

		m_primitiveRenderer->end(m_renderView);
	}

	if (m_effectInstance)
	{
		float deltaTime = float(m_timer.getDeltaTime() * 0.2f + m_lastDeltaTime * 0.8f);

		m_context.deltaTime = deltaTime * m_timeScale;

		m_effectInstance->update(m_context, Matrix44::identity(), true);
		m_effectInstance->synchronize();
		m_effectInstance->render(m_pointRenderer, cameraPlane);

		Frustum viewFrustum;
		viewFrustum.buildPerspective(80.0f * PI / 180.0f, aspect, 0.1f, 2000.0f);

		world::WorldRenderView worldRenderView;
		worldRenderView.setTechnique(render::getParameterHandle(L"Default"));
		worldRenderView.setProjection(projectionTransform);
		worldRenderView.setView(viewTransform);
		worldRenderView.setViewSize(Vector2(float(viewport.width), float(viewport.height)));
		worldRenderView.setCullFrustum(viewFrustum);
		worldRenderView.setViewFrustum(viewFrustum);

		world::WorldRenderView::Light globalLight;
		globalLight.type = world::WorldRenderView::LtDirectional;
		globalLight.position = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		globalLight.direction = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
		globalLight.sunColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		globalLight.baseColor = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
		globalLight.shadowColor = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		globalLight.range = Scalar(0.0f);
		worldRenderView.addLight(globalLight);

		m_pointRenderer->flush(
			m_renderContext,
			&worldRenderView
		);

		m_renderContext->render(render::RenderContext::RfOpaque | render::RenderContext::RfAlphaBlend);
		m_renderContext->flush();

		//if (m_velocityVisible)
		//{
		//	const PointVector& points = m_effectInstance->getPoints();

		//	m_primitiveRenderer->pushProjection(projectionTransform);
		//	m_primitiveRenderer->pushView(viewTransform);

		//	for (PointVector::const_iterator i = points.begin(); i != points.end(); ++i)
		//		m_primitiveRenderer->drawLine(i->position, i->position + i->velocity / Scalar(10.0f), Color(255, 255, 128));
		//	
		//	m_primitiveRenderer->flush(m_renderView);
		//}

		m_lastDeltaTime = deltaTime;
	}

	m_renderView->end();
	m_renderView->present();

	event->consume();
}

void EffectPreviewControl::eventIdle(ui::Event* event)
{
	ui::IdleEvent* idleEvent = checked_type_cast< ui::IdleEvent* >(event);
	if (isVisible(true))
	{
		update();
		idleEvent->requestMore();
	}
}

	}
}
