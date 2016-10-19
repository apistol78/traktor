#include <ctime>
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/ISimpleTexture.h"
#include "Render/PrimitiveRenderer.h"
#include "Render/RenderTargetSet.h"
#include "Render/Context/RenderContext.h"
#include "Render/ImageProcess/ImageProcess.h"
#include "Render/ImageProcess/ImageProcessSettings.h"
#include "Resource/IResourceManager.h"
#include "Sound/SoundSystem.h"
#include "Sound/Player/SoundPlayer.h"
#include "Spray/Effect.h"
#include "Spray/EffectData.h"
#include "Spray/EffectInstance.h"
#include "Spray/EffectLayer.h"
#include "Spray/EffectLayerData.h"
#include "Spray/EffectLayerInstance.h"
#include "Spray/Emitter.h"
#include "Spray/EmitterData.h"
#include "Spray/EmitterInstance.h"
#include "Spray/MeshRenderer.h"
#include "Spray/PointRenderer.h"
#include "Spray/SourceData.h"
#include "Spray/TrailRenderer.h"
#include "Spray/Editor/BoxSourceRenderer.h"
#include "Spray/Editor/ConeSourceRenderer.h"
#include "Spray/Editor/DiscSourceRenderer.h"
#include "Spray/Editor/EffectPreviewControl.h"
#include "Spray/Editor/LineSourceRenderer.h"
#include "Spray/Editor/PointSourceRenderer.h"
#include "Spray/Editor/PointSetSourceRenderer.h"
#include "Spray/Editor/QuadSourceRenderer.h"
#include "Spray/Editor/SphereSourceRenderer.h"
#include "Spray/Sources/BoxSourceData.h"
#include "Spray/Sources/ConeSourceData.h"
#include "Spray/Sources/DiscSourceData.h"
#include "Spray/Sources/LineSourceData.h"
#include "Spray/Sources/PointSourceData.h"
#include "Spray/Sources/PointSetSourceData.h"
#include "Spray/Sources/QuadSourceData.h"
#include "Spray/Sources/SphereSourceData.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Application.h"
#include "Ui/PopupMenu.h"
#include "Ui/MenuItem.h"
#include "World/WorldRenderView.h"
#include "World/Forward/WorldRenderPassForward.h"

namespace traktor
{
	namespace spray
	{
		namespace
		{

const uint32_t c_initialRandomSeed = 5489UL;
const int c_updateInterval = 30;
const float c_deltaMoveScale = 0.025f;
const float c_deltaScaleHead = 0.015f;
const float c_deltaScalePitch = 0.005f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectPreviewControl", EffectPreviewControl, ui::Widget)

EffectPreviewControl::EffectPreviewControl(editor::IEditor* editor)
:	m_editor(editor)
,	m_randomSeed(c_initialRandomSeed)
,	m_effectPosition(0.0f, -2.0f, 7.0f, 1.0f)
,	m_angleHead(0.0f)
,	m_anglePitch(0.0f)
,	m_timeScale(1.0f)
,	m_lastDeltaTime(1.0f / c_updateInterval)
,	m_guideVisible(true)
,	m_velocityVisible(false)
,	m_moveEmitter(false)
,	m_groundClip(false)
{
	// Allocate "global" parameter context; as it's reset for each render
	// call this can be fairly small.
	m_globalContext = new render::RenderContext(4096);

	m_context.deltaTime = 0.0f;
	m_context.random = RandomGeometry(c_initialRandomSeed);
	m_context.eventManager = 0;
	m_context.soundPlayer = 0;

	m_sourceRenderers[&type_of< BoxSourceData >()] = new BoxSourceRenderer();
	m_sourceRenderers[&type_of< ConeSourceData >()] = new ConeSourceRenderer();
	m_sourceRenderers[&type_of< DiscSourceData >()] = new DiscSourceRenderer();
	m_sourceRenderers[&type_of< LineSourceData >()] = new LineSourceRenderer();
	m_sourceRenderers[&type_of< PointSourceData >()] = new PointSourceRenderer();
	m_sourceRenderers[&type_of< PointSetSourceData >()] = new PointSetSourceRenderer();
	m_sourceRenderers[&type_of< QuadSourceData >()] = new QuadSourceRenderer();
	m_sourceRenderers[&type_of< SphereSourceData >()] = new SphereSourceRenderer();
}

bool EffectPreviewControl::create(
	ui::Widget* parent,
	int style,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	sound::SoundSystem* soundSystem
)
{
	if (!Widget::create(parent, style))
		return false;

	ui::Rect innerRect = getInnerRect();
	int32_t width = innerRect.getWidth();
	int32_t height = innerRect.getHeight();

	m_resourceManager = resourceManager;
	m_renderSystem = renderSystem;

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 32;
	desc.stencilBits = 0;
	desc.multiSample = 0;
	desc.waitVBlanks = 0;
	desc.syswin = getIWidget()->getSystemWindow();

	m_renderView = renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	if (width > 0 && height > 0)
	{
		render::RenderTargetSetCreateDesc rtscd;
		rtscd.count = 1;
		rtscd.width = width;
		rtscd.height = height;
		rtscd.multiSample = desc.multiSample;
		rtscd.createDepthStencil = false;
		rtscd.usingPrimaryDepthStencil = true;
		rtscd.preferTiled = false;
		rtscd.ignoreStencil = true;
		rtscd.generateMips = false;
		rtscd.targets[0].format = render::TfR32F;
		rtscd.targets[0].sRGB = false;

		m_depthTexture = renderSystem->createRenderTargetSet(rtscd);
		if (!m_depthTexture)
			return false;
	}

	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(resourceManager, renderSystem))
		return false;

	m_renderContext = new render::RenderContext(512 * 1024);

	if ((m_soundSystem = soundSystem) != 0)
	{
		m_soundPlayer = new sound::SoundPlayer();
		m_soundPlayer->create(m_soundSystem, 0);
	}

	m_pointRenderer = new PointRenderer(renderSystem, 50.0f, 100.0f);
	m_meshRenderer = new MeshRenderer();
	m_trailRenderer = new TrailRenderer(renderSystem);

	addEventHandler< ui::MouseButtonDownEvent >(this, &EffectPreviewControl::eventButtonDown);
	addEventHandler< ui::MouseButtonUpEvent >(this, &EffectPreviewControl::eventButtonUp);
	addEventHandler< ui::MouseMoveEvent >(this, &EffectPreviewControl::eventMouseMove);
	addEventHandler< ui::SizeEvent >(this, &EffectPreviewControl::eventSize);
	addEventHandler< ui::PaintEvent >(this, &EffectPreviewControl::eventPaint);

	updateSettings();
	m_timer.start();

	m_idleEventHandler = ui::Application::getInstance()->addEventHandler< ui::IdleEvent >(this, &EffectPreviewControl::eventIdle);

	return true;
}

void EffectPreviewControl::destroy()
{
	ui::Application::getInstance()->removeEventHandler< ui::IdleEvent >(m_idleEventHandler);

	safeDestroy(m_primitiveRenderer);
	safeDestroy(m_trailRenderer);
	safeDestroy(m_meshRenderer);
	safeDestroy(m_pointRenderer);

	safeDestroy(m_depthTexture);
	safeClose(m_renderView);

	safeDestroy(m_soundPlayer);
	m_soundSystem = 0;

	Widget::destroy();
}

void EffectPreviewControl::setEffect(const EffectData* effectData, Effect* effect)
{
	m_effectData = effectData;
	if ((m_effect = effect) != 0)
		m_effectInstance = m_effect->createInstance();
	else
		m_effectInstance = 0;
}

uint32_t EffectPreviewControl::getEffectLayerPoints(const EffectLayer* effectLayer) const
{
	if (m_effectInstance)
	{
		const RefArray< EffectLayerInstance >& layerInstances = m_effectInstance->getLayerInstances();
		for (RefArray< EffectLayerInstance >::const_iterator i = layerInstances.begin(); i != layerInstances.end(); ++i)
		{
			if ((*i)->getLayer() == effectLayer)
			{
				const EmitterInstance* emitterInstance = (*i)->getEmitterInstance();
				if (emitterInstance)
					return emitterInstance->getPoints().size();
				else
					return 0;
			}
		}
	}
	return 0;
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

void EffectPreviewControl::setBackground(const resource::Id< render::ISimpleTexture >& background)
{
	m_resourceManager->bind(background, m_background);
}

void EffectPreviewControl::setImageProcess(const resource::Id< render::ImageProcessSettings >& postProcess)
{
	m_resourceManager->bind(postProcess, m_postProcessSettings);
	updateRenderer();
}

void EffectPreviewControl::showGuide(bool guideVisible)
{
	m_guideVisible = guideVisible;
}

void EffectPreviewControl::showVelocity(bool velocityVisible)
{
	m_velocityVisible = velocityVisible;
}

void EffectPreviewControl::setMoveEmitter(bool moveEmitter)
{
	m_moveEmitter = moveEmitter;
}

void EffectPreviewControl::setGroundClip(bool groundClip)
{
	m_groundClip = groundClip;
}

void EffectPreviewControl::randomizeSeed()
{
	m_randomSeed = clock();
	m_context.random = RandomGeometry(m_randomSeed);
}

void EffectPreviewControl::syncEffect()
{
	if (!m_effect || !m_effectInstance)
		return;

	Context syncContext;
	syncContext.deltaTime = 0.0f;
	syncContext.random = RandomGeometry(m_randomSeed);
	syncContext.eventManager = 0;
	syncContext.soundPlayer = 0;

	float currentTime = m_effectInstance->getTime();

	// Re-create instance.
	m_effectInstance = m_effect->createInstance();

	// Run emitter until total time is reached.
	const float c_deltaTime = 1.0f / 30.0f;
	for (float T = 0.0f; T < currentTime; T += c_deltaTime)
	{
		Transform effectTransform = Transform::identity();
		if (m_moveEmitter)
		{
			Vector4 effectPosition(
				std::sin(T) * 8.0f,
				0.0f,
				std::cos(T) * 8.0f,
				1.0f
			);
			effectTransform = Transform(effectPosition);
		}

		float deltaTime = min(c_deltaTime, currentTime - T);
		syncContext.deltaTime = deltaTime;

		m_effectInstance->update(syncContext, effectTransform, true);
		m_effectInstance->synchronize();
	}

	m_context = syncContext;
	update();
}

void EffectPreviewControl::updateSettings()
{
	Ref< PropertyGroup > colors = m_editor->getSettings()->getProperty< PropertyGroup >(L"Editor.Colors");
	m_colorClear = colors->getProperty< PropertyColor >(L"Background");
	m_colorGrid = colors->getProperty< PropertyColor >(L"Grid");
}

void EffectPreviewControl::updateRenderer()
{
	safeDestroy(m_depthTexture);
	safeDestroy(m_postTargetSet);
	safeDestroy(m_postProcess);

	if (!m_renderView)
		return;

	ui::Size sz = getInnerRect().getSize();

	m_renderView->reset(sz.cx, sz.cy);
	m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));

	render::RenderTargetSetCreateDesc rtscd;
	rtscd.count = 1;
	rtscd.width = sz.cx;
	rtscd.height = sz.cy;
	rtscd.multiSample = 0/*desc.multiSample*/;
	rtscd.createDepthStencil = false;
	rtscd.usingPrimaryDepthStencil = true;
	rtscd.preferTiled = false;
	rtscd.ignoreStencil = true;
	rtscd.generateMips = false;
	rtscd.targets[0].format = render::TfR32F;
	rtscd.targets[0].sRGB = false;

	m_depthTexture = m_renderSystem->createRenderTargetSet(rtscd);

	// Re-create post processing.
	if (m_postProcessSettings)
	{
		m_postProcess = new render::ImageProcess();
		m_postProcess->create(m_postProcessSettings, 0, m_resourceManager, m_renderSystem, sz.cx, sz.cy, false);

		render::RenderTargetSetCreateDesc desc;
		desc.count = 1;
		desc.width = sz.cx;
		desc.height = sz.cy;
		desc.multiSample = 0;
		desc.createDepthStencil = false;
		desc.usingPrimaryDepthStencil = true;
		desc.preferTiled = false;
		desc.ignoreStencil = false;
		desc.generateMips = false;
		desc.targets[0].format = render::TfR11G11B10F;
		desc.targets[0].sRGB = false;
		m_postTargetSet = m_renderSystem->createRenderTargetSet(desc);
	}
}

void EffectPreviewControl::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	m_lastMousePosition = event->getPosition();
	setCapture();
}

void EffectPreviewControl::eventButtonUp(ui::MouseButtonUpEvent* event)
{
	releaseCapture();
}

void EffectPreviewControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	if (!hasCapture())
		return;

	if ((event->getKeyState() & ui::KsMenu) != 0)
	{
		if (event->getButton() == ui::MbtRight)
		{
			if ((event->getKeyState() & ui::KsControl) == 0)
			{
				// Move X/Z direction.
				float dx = -float(m_lastMousePosition.x - event->getPosition().x) * c_deltaMoveScale;
				float dz = -float(m_lastMousePosition.y - event->getPosition().y) * c_deltaMoveScale;
				m_effectPosition += Vector4(dx, 0.0f, dz, 0.0f);
			}
			else
			{
				// Move X/Y direction.
				float dx = -float(m_lastMousePosition.x - event->getPosition().x) * c_deltaMoveScale;
				float dy =  float(m_lastMousePosition.y - event->getPosition().y) * c_deltaMoveScale;
				m_effectPosition += Vector4(dx, dy, 0.0f, 0.0f);
			}
		}
		else if (event->getButton() == ui::MbtLeft)
		{
			m_angleHead += float(m_lastMousePosition.x - event->getPosition().x) * c_deltaScaleHead;
			m_anglePitch += float(m_lastMousePosition.y - event->getPosition().y) * c_deltaScalePitch;
		}
	}

	m_lastMousePosition = event->getPosition();

	update();
}

void EffectPreviewControl::eventSize(ui::SizeEvent* event)
{
	updateRenderer();
}

void EffectPreviewControl::eventPaint(ui::PaintEvent* event)
{
	float time = float(m_timer.getElapsedTime());
	float deltaTime = float(m_timer.getDeltaTime() * 0.9f + m_lastDeltaTime * 0.1f);

	if (!m_renderView)
		return;

	if (!m_renderView->begin(render::EtCyclop))
		return;

	float tmp[4];
	m_colorClear.getRGBA32F(tmp);
	Color4f clearColor(tmp[0], tmp[1], tmp[2], tmp[3]);

	m_renderView->clear(
		render::CfColor | render::CfDepth,
		&clearColor,
		1.0f,
		128
	);

	render::Viewport viewport = m_renderView->getViewport();
	float aspect = float(viewport.width) / viewport.height;

	Matrix44 viewTransform = translate(m_effectPosition) * rotateX(m_anglePitch) * rotateY(m_angleHead);
	Matrix44 projectionTransform = perspectiveLh(
		65.0f * PI / 180.0f,
		aspect,
		0.01f,
		1000.0f
	);

	Matrix44 viewInverse = viewTransform.inverse();

	Vector4 cameraPosition = viewInverse.translation().xyz1();
	Plane cameraPlane(viewInverse.axisZ(), viewInverse.translation());

	if (!m_primitiveRenderer->begin(m_renderView, Matrix44::identity()))
		return;

	if (m_background)
	{
		m_primitiveRenderer->pushView(Matrix44::identity());
		m_primitiveRenderer->pushDepthState(false, false, false);

		m_primitiveRenderer->drawTextureQuad(
			Vector4(-1.0f,  1.0f, 1.0f, 1.0f), Vector2(0.0f, 0.0f),
			Vector4( 1.0f,  1.0f, 1.0f, 1.0f), Vector2(1.0f, 0.0f),
			Vector4( 1.0f, -1.0f, 1.0f, 1.0f), Vector2(1.0f, 1.0f),
			Vector4(-1.0f, -1.0f, 1.0f, 1.0f), Vector2(0.0f, 1.0f),
			Color4ub(255, 255, 255, 255),
			m_background
		);

		m_primitiveRenderer->popDepthState();
		m_primitiveRenderer->popView();
	}

	// Draw depth-only ground plane; primary depth and slight tint.
	if (m_groundClip)
	{
		m_primitiveRenderer->pushDepthState(true, true, false);
		m_primitiveRenderer->drawSolidQuad(
			Vector4(-1000.0f, 0.0f, -1000.0f, 1.0f),
			Vector4( 1000.0f, 0.0f, -1000.0f, 1.0f),
			Vector4( 1000.0f, 0.0f,  1000.0f, 1.0f),
			Vector4(-1000.0f, 0.0f,  1000.0f, 1.0f),
			Color4ub(0, 0, 0, 10)
		);
		m_primitiveRenderer->popDepthState();
	}

	m_primitiveRenderer->end();

	// Draw depth-only ground plane.
	if (m_depthTexture && m_renderView->begin(m_depthTexture, 0))
	{
		const float farZ = 10000.0f;
		const Color4f clearColor(farZ, farZ, farZ, farZ);
		m_renderView->clear(render::CfColor, &clearColor, 1.0f, 0);

		if (m_groundClip && m_primitiveRenderer->begin(m_renderView, projectionTransform))
		{
			m_primitiveRenderer->pushView(viewTransform);

			m_primitiveRenderer->pushDepthState(true, true, true);
			m_primitiveRenderer->drawSolidQuad(
				Vector4(-1000.0f, 0.0f, -1000.0f, 1.0f),
				Vector4( 1000.0f, 0.0f, -1000.0f, 1.0f),
				Vector4( 1000.0f, 0.0f,  1000.0f, 1.0f),
				Vector4(-1000.0f, 0.0f,  1000.0f, 1.0f),
				Color4ub(0, 0, 0, 0)
			);
			m_primitiveRenderer->popDepthState();

			m_primitiveRenderer->popView();
			m_primitiveRenderer->end();
		}

		m_renderView->end();
	}

	if (m_postProcess)
	{
		if (m_renderView->begin(m_postTargetSet, 0))
			m_renderView->clear(render::CfColor, &clearColor, 1.0f, 0);
	}

	if (m_effectInstance)
	{
		m_context.deltaTime = deltaTime * m_timeScale;
		m_context.soundPlayer = m_soundPlayer;

		Transform effectTransform = Transform::identity();
		if (m_moveEmitter)
		{
			Vector4 effectPosition(
				std::sin(time) * 8.0f,
				0.0f,
				std::cos(time) * 8.0f,
				1.0f
			);
			Vector4 effectDirection(
				std::cos(time),
				0.0f,
				-std::sin(time),
				0.0f
			);
			effectTransform = Transform(
				effectPosition,
				Quaternion(
					Vector4(0.0f, 0.0f, 1.0f),
					effectDirection
				)
			);
		}

		m_effectInstance->update(m_context, effectTransform, true);
		m_effectInstance->synchronize();
		m_effectInstance->render(
			render::getParameterHandle(L"World_ForwardColor"),
			m_pointRenderer,
			m_meshRenderer,
			m_trailRenderer,
			Transform::identity(),
			cameraPosition,
			cameraPlane
		);

		Frustum viewFrustum;
		viewFrustum.buildPerspective(80.0f * PI / 180.0f, aspect, 0.1f, 2000.0f);

		world::WorldRenderView worldRenderView;
		worldRenderView.setProjection(projectionTransform);
		worldRenderView.setView(viewTransform);
		worldRenderView.setViewSize(Vector2(float(viewport.width), float(viewport.height)));
		worldRenderView.setCullFrustum(viewFrustum);
		worldRenderView.setViewFrustum(viewFrustum);
		worldRenderView.setTimes(time, deltaTime, 0.0f);

		world::Light globalLight;
		globalLight.type = world::LtDirectional;
		globalLight.position = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		globalLight.direction = Vector4(0.0f, 1.0f, -1.0f, 0.0f).normalized();
		globalLight.sunColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		globalLight.baseColor = Vector4(0.75f, 0.75f, 0.75f, 1.0f);
		globalLight.shadowColor = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
		globalLight.range = Scalar(0.0f);
		worldRenderView.addLight(globalLight);

		world::WorldRenderPassForward defaultPass(
			render::getParameterHandle(L"World_ForwardColor"),
			worldRenderView,
			true,
			m_background,
			m_depthTexture->getColorTexture(0)
		);

		m_pointRenderer->flush(m_renderContext, defaultPass);
		m_meshRenderer->flush(m_renderContext, defaultPass);
		m_trailRenderer->flush(m_renderContext, defaultPass);

		render::ProgramParameters visualProgramParams;
		visualProgramParams.beginParameters(m_globalContext);
		visualProgramParams.setFloatParameter(L"World_Time", time);
		visualProgramParams.setMatrixParameter(L"World_View", viewTransform);
		visualProgramParams.setMatrixParameter(L"World_Projection", projectionTransform);
		visualProgramParams.endParameters(m_globalContext);

		m_renderContext->render(m_renderView, render::RpAll, &visualProgramParams);
		m_renderContext->flush();

		m_globalContext->flush();

		m_lastDeltaTime = deltaTime;
	}

	if (m_postProcess)
	{
		m_renderView->end();

		render::ImageProcessStep::Instance::RenderParams params;
		params.view = viewTransform;
		params.viewToLight = Matrix44::identity();
		params.projection = projectionTransform;
		params.godRayDirection = Vector4(0.0f, 0.0f, -1.0f);
		params.sliceCount = 0;
		params.sliceIndex = 0;
		params.sliceNearZ = 0.0f;
		params.sliceFarZ = 0.0f;
		params.shadowFarZ = 0.0f;
		params.shadowMapBias = 0.0f;
		params.deltaTime = deltaTime;
		m_postProcess->render(
			m_renderView,
			m_postTargetSet,
			0,
			0,
			params
		);
	}

	if (!m_primitiveRenderer->begin(m_renderView, projectionTransform))
		return;

	m_primitiveRenderer->pushView(viewTransform);

	for (int x = -10; x <= 10; ++x)
	{
		m_primitiveRenderer->drawLine(
			Vector4(float(x), 0.0f, -10.0f, 1.0f),
			Vector4(float(x), 0.0f, 10.0f, 1.0f),
			(x == 0) ? 2.0f : 0.0f,
			m_colorGrid
		);
		m_primitiveRenderer->drawLine(
			Vector4(-10.0f, 0.0f, float(x), 1.0f),
			Vector4(10.0f, 0.0f, float(x), 1.0f),
			(x == 0) ? 2.0f : 0.0f,
			m_colorGrid
		);
	}

	// Draw emitter sources.
	if (m_effectData && m_guideVisible)
	{
		const RefArray< EffectLayerData >& layers = m_effectData->getLayers();
		for (RefArray< EffectLayerData >::const_iterator i = layers.begin(); i != layers.end(); ++i)
		{
			Ref< const EmitterData > emitterData = (*i)->getEmitter();
			if (!emitterData)
				continue;

			Ref< const SourceData > sourceData = emitterData->getSource();
			if (!sourceData)
				continue;

			std::map< const TypeInfo*, Ref< SourceRenderer > >::const_iterator j = m_sourceRenderers.find(&type_of(sourceData));
			if (j != m_sourceRenderers.end())
				j->second->render(m_primitiveRenderer, sourceData);
		}
	}

	m_primitiveRenderer->end();

	m_renderView->end();
	m_renderView->present();

	event->consume();
}

void EffectPreviewControl::eventIdle(ui::IdleEvent* event)
{
	if (isVisible(true))
	{
		update();
		event->requestMore();
	}
}

	}
}
