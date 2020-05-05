#include <ctime>
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Mesh/MeshComponentRenderer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Resource/IResourceManager.h"
#include "Scene/Scene.h"
#include "Sound/AudioSystem.h"
#include "Sound/Player/SoundPlayer.h"
#include "Spray/Effect.h"
#include "Spray/EffectComponent.h"
#include "Spray/EffectData.h"
#include "Spray/EffectEntityRenderer.h"
#include "Spray/EffectInstance.h"
#include "Spray/EffectLayer.h"
#include "Spray/EffectLayerData.h"
#include "Spray/EffectLayerInstance.h"
#include "Spray/Emitter.h"
#include "Spray/EmitterData.h"
#include "Spray/EmitterInstance.h"
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
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Weather/WeatherRenderer.h"
#include "World/EntityRenderer.h"
#include "World/IWorldRenderer.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity/GroupEntity.h"
#include "World/Entity/GroupRenderer.h"
#include "World/Entity/LightRenderer.h"
#include "World/Entity/ProbeRenderer.h"

namespace traktor
{
	namespace spray
	{
		namespace
		{

const resource::Id< scene::Scene > c_previewScene(L"{473467B0-835D-EF45-B308-E3C3C5B0F226}");

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
{
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
	sound::AudioSystem* audioSystem
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

	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(resourceManager, renderSystem, 1))
		return false;

	m_renderContext = new render::RenderContext(16 * 1024 * 1024);
	m_renderGraph = new render::RenderGraph(renderSystem);

	if ((m_audioSystem = audioSystem) != nullptr)
	{
		m_soundPlayer = new sound::SoundPlayer();
		m_soundPlayer->create(m_audioSystem, nullptr);
	}

	if (!resourceManager->bind(c_previewScene, m_sceneInstance))
		return false;

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
	safeClose(m_renderView);

	safeDestroy(m_soundPlayer);
	m_audioSystem = nullptr;

	Widget::destroy();
}

void EffectPreviewControl::setEffect(const EffectData* effectData, Effect* effect)
{
	m_effectData = effectData;
	if ((m_effect = effect) != nullptr)
	{
		m_effectEntity = new world::Entity();
		m_effectEntity->setComponent(new EffectComponent(
			resource::Proxy< Effect >(m_effect),
			nullptr,
			m_soundPlayer
		));
	}
	else
		m_effectEntity = nullptr;
}

uint32_t EffectPreviewControl::getEffectLayerPoints(const EffectLayer* effectLayer) const
{
	if (!m_effectEntity)
		return 0;

	auto effectComponent = m_effectEntity->getComponent< EffectComponent >();
	T_ASSERT(effectComponent != nullptr);

	auto effectInstance = effectComponent->getEffectInstance();
	if (!effectInstance)
		return 0;

	for (auto layerInstance : effectInstance->getLayerInstances())
	{
		if (layerInstance->getLayer() == effectLayer)
		{
			auto emitterInstance = layerInstance->getEmitterInstance();
			if (emitterInstance)
				return (uint32_t)emitterInstance->getPoints().size();
			else
				return 0;
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
	if (!m_effectEntity)
		return;

	auto effectComponent = m_effectEntity->getComponent< EffectComponent >();
	T_ASSERT(effectComponent != nullptr);

	auto effectInstance = effectComponent->getEffectInstance();
	if (!effectInstance)
		return;

	effectInstance->setTime(totalTime);
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

void EffectPreviewControl::randomizeSeed()
{
	m_randomSeed = clock();
}

void EffectPreviewControl::syncEffect()
{
	if (!m_effectEntity)
		return;

	auto effectComponent = m_effectEntity->getComponent< EffectComponent >();
	T_ASSERT(effectComponent != nullptr);

	float currentTime = 0.0f;

	// Get time from current effect instance.
	{
		auto effectInstance = effectComponent->getEffectInstance();
		if (effectInstance)
			currentTime = effectInstance->getTime();
	}
	
	Context context;
	context.deltaTime = 0.0f;
	context.random = RandomGeometry(m_randomSeed);
	context.eventManager = nullptr;
	context.soundPlayer = nullptr;

	// Create new effect instance.
	Ref< EffectInstance > effectInstance = effectComponent->getEffect()->createInstance();

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
		context.deltaTime = deltaTime;

		effectInstance->update(context, effectTransform, true);
		effectInstance->synchronize();
	}

	context.eventManager = nullptr;
	context.soundPlayer = m_soundPlayer;

	m_effectEntity->setComponent(new EffectComponent(
		effectComponent->getEffect(),
		effectInstance,
		context
	));

	update();
}

void EffectPreviewControl::updateSettings()
{
	Ref< PropertyGroup > colors = m_editor->getSettings()->getProperty< PropertyGroup >(L"Editor.Colors");
	m_colorClear = colors->getProperty< Color4ub >(L"Background");
	m_colorGrid = colors->getProperty< Color4ub >(L"Grid");
}

void EffectPreviewControl::updateWorldRenderer()
{
	safeDestroy(m_worldRenderer);

	if (!m_sceneInstance)
		return;

	std::wstring worldRendererTypeName = L"traktor.world.WorldRendererDeferred"; // settings->getProperty< std::wstring >(L"SceneEditor.WorldRendererType", L"traktor.world.WorldRendererDeferred");

	const TypeInfo* worldRendererType = TypeInfo::find(worldRendererTypeName.c_str());
	if (!worldRendererType)
		return;

	// Create entity renderers.
	Ref< world::WorldEntityRenderers > entityRenderers = new world::WorldEntityRenderers();
	entityRenderers->add(new mesh::MeshComponentRenderer());
	entityRenderers->add(new EffectEntityRenderer(m_renderSystem, 0.0f, 1000.0f));
	entityRenderers->add(new weather::WeatherRenderer());
	entityRenderers->add(new world::EntityRenderer());
	entityRenderers->add(new world::GroupRenderer());
	entityRenderers->add(new world::LightRenderer());
	entityRenderers->add(new world::ProbeRenderer(
		m_resourceManager,
		m_renderSystem,
		*worldRendererType
	));

	Ref< world::IWorldRenderer > worldRenderer = dynamic_type_cast< world::IWorldRenderer* >(worldRendererType->createInstance());
	if (!worldRenderer)
		return;

	world::WorldCreateDesc wcd;
	wcd.worldRenderSettings = m_sceneInstance->getWorldRenderSettings();
	wcd.entityRenderers = entityRenderers;
	wcd.quality.motionBlur = world::Quality::Disabled;
	wcd.quality.shadows = world::Quality::Ultra;
	wcd.quality.reflections = world::Quality::Ultra;
	wcd.quality.ambientOcclusion = world::Quality::Ultra;
	wcd.quality.antiAlias = world::Quality::Ultra;
	wcd.quality.imageProcess = world::Quality::Ultra;
	wcd.multiSample = 0;
	wcd.frameCount = 1;

	if (!worldRenderer->create(
		m_resourceManager,
		m_renderSystem,
		wcd
	))
		return;

	m_worldRenderer = worldRenderer;
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
	if (!m_renderView)
		return;

	ui::Size sz = getInnerRect().getSize();

	m_renderView->reset(sz.cx, sz.cy);
	m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));
}

void EffectPreviewControl::eventPaint(ui::PaintEvent* event)
{
	// Reload scene if changed.
	if (m_sceneInstance.changed())
	{
		m_worldRenderer = nullptr;
		m_sceneInstance.consume();
	}

	if (!m_sceneInstance || !m_renderView)
		return;

	// Lazy create world renderer.
	if (!m_worldRenderer)
	{
		updateWorldRenderer();
		if (!m_worldRenderer)
			return;
	}

	ui::Size sz = getInnerRect().getSize();

	float time = (float)m_timer.getElapsedTime();
	float deltaTime = (float)(m_timer.getDeltaTime() * 0.9f + m_lastDeltaTime * 0.1f);

	float tmp[4];
	m_colorClear.getRGBA32F(tmp);

	Matrix44 view = translate(m_effectPosition) * rotateX(m_anglePitch) * rotateY(m_angleHead);

	// Update scene entities.
	world::UpdateParams update;
	update.totalTime = time;
	update.deltaTime = deltaTime * m_timeScale;
	update.alternateTime = time;
	m_sceneInstance->updateController(update);
	m_sceneInstance->updateEntity(update);

	// Build a root entity by gathering entities from containers.
	world::GroupEntity rootEntity;
	// m_context->getEntityEventManager()->gather([&](world::Entity* entity) { rootEntity.addEntity(entity); });
	rootEntity.addEntity(m_sceneInstance->getRootEntity());
	if (m_effectEntity)
	{
		m_effectEntity->update(update);
		rootEntity.addEntity(m_effectEntity);
	}

	// Setup world render passes.
	const world::WorldRenderSettings* worldRenderSettings = m_sceneInstance->getWorldRenderSettings();
	m_worldRenderView.setPerspective(
		float(sz.cx),
		float(sz.cy),
		float(sz.cx) / sz.cy,
		deg2rad(70.0f), // m_fieldOfView),
		worldRenderSettings->viewNearZ,
		worldRenderSettings->viewFarZ
	);
	m_worldRenderView.setTimes(time, deltaTime, 1.0f);
	m_worldRenderView.setView(m_worldRenderView.getView(), view);
	m_worldRenderer->setup(m_worldRenderView, &rootEntity, *m_renderGraph, 0);

	// Draw debug wires.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Debug wire");
	rp->setOutput(0);
	rp->addBuild([&](const render::RenderGraph&, render::RenderContext* renderContext) {
		m_primitiveRenderer->begin(0, m_worldRenderView.getProjection());
		m_primitiveRenderer->pushView(view);

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

		if (m_effectData && m_guideVisible)
		{
			for (auto layer : m_effectData->getLayers())
			{
				const EmitterData* emitterData = layer->getEmitter();
				if (!emitterData)
					continue;

				const SourceData* sourceData = emitterData->getSource();
				if (!sourceData)
					continue;

				auto it = m_sourceRenderers.find(&type_of(sourceData));
				if (it != m_sourceRenderers.end())
					it->second->render(m_primitiveRenderer, sourceData);
			}
		}

		m_primitiveRenderer->end(0);

		auto rb = renderContext->alloc< render::LambdaRenderBlock >();
		rb->lambda = [&](render::IRenderView* renderView) {
			m_primitiveRenderer->render(m_renderView, 0);
		};
		renderContext->enqueue(rb);		
	});
	m_renderGraph->addPass(rp);

	// Validate render graph.
	if (!m_renderGraph->validate())
		return;

	// Build render context.
	m_renderContext->flush();
	m_renderGraph->build(m_renderContext, sz.cx, sz.cy);

	// Render frame.
	if (m_renderView->beginFrame())
	{
		m_renderContext->render(m_renderView);
		m_renderView->endFrame();
		m_renderView->present();
	}


	// if (!m_renderView->beginPass(&cl))
	// 	return;

	// ui::Size sz = getInnerRect().getSize();
	// float aspect = float(sz.cx) / sz.cy;

	// Matrix44 viewTransform = translate(m_effectPosition) * rotateX(m_anglePitch) * rotateY(m_angleHead);
	// Matrix44 projectionTransform = perspectiveLh(
	// 	65.0f * PI / 180.0f,
	// 	aspect,
	// 	0.01f,
	// 	1000.0f
	// );

	// Matrix44 viewInverse = viewTransform.inverse();

	// Vector4 cameraPosition = viewInverse.translation().xyz1();
	// Plane cameraPlane(viewInverse.axisZ(), viewInverse.translation());

	// if (!m_primitiveRenderer->begin(0, Matrix44::identity()))
	// 	return;

	// if (m_background)
	// {
	// 	m_primitiveRenderer->pushView(Matrix44::identity());
	// 	m_primitiveRenderer->pushDepthState(false, false, false);

	// 	m_primitiveRenderer->drawTextureQuad(
	// 		Vector4(-1.0f,  1.0f, 1.0f, 1.0f), Vector2(0.0f, 0.0f),
	// 		Vector4( 1.0f,  1.0f, 1.0f, 1.0f), Vector2(1.0f, 0.0f),
	// 		Vector4( 1.0f, -1.0f, 1.0f, 1.0f), Vector2(1.0f, 1.0f),
	// 		Vector4(-1.0f, -1.0f, 1.0f, 1.0f), Vector2(0.0f, 1.0f),
	// 		Color4ub(255, 255, 255, 255),
	// 		m_background
	// 	);

	// 	m_primitiveRenderer->popDepthState();
	// 	m_primitiveRenderer->popView();
	// }

	// // Draw depth-only ground plane; primary depth and slight tint.
	// if (m_groundClip)
	// {
	// 	m_primitiveRenderer->pushDepthState(true, true, false);
	// 	m_primitiveRenderer->drawSolidQuad(
	// 		Vector4(-1000.0f, 0.0f, -1000.0f, 1.0f),
	// 		Vector4( 1000.0f, 0.0f, -1000.0f, 1.0f),
	// 		Vector4( 1000.0f, 0.0f,  1000.0f, 1.0f),
	// 		Vector4(-1000.0f, 0.0f,  1000.0f, 1.0f),
	// 		Color4ub(0, 0, 0, 10)
	// 	);
	// 	m_primitiveRenderer->popDepthState();
	// }

	// m_primitiveRenderer->end(0);
	// m_primitiveRenderer->render(m_renderView, 0);

	// // Draw depth-only ground plane.
	// const float farZ = 10000.0f;

	// cl.mask = render::CfColor;
	// cl.colors[0] = Color4f(farZ, farZ, farZ, farZ);
	// cl.depth = 1.0f;
	// cl.stencil = 0;

	// if (
	// 	m_depthTexture/* &&
	// 	m_renderView->beginPass(m_depthTexture, 0, &cl)*/
	// )
	// {
	// 	if (m_groundClip && m_primitiveRenderer->begin(0, projectionTransform))
	// 	{
	// 		m_primitiveRenderer->pushView(viewTransform);

	// 		m_primitiveRenderer->pushDepthState(true, true, true);
	// 		m_primitiveRenderer->drawSolidQuad(
	// 			Vector4(-1000.0f, 0.0f, -1000.0f, 1.0f),
	// 			Vector4( 1000.0f, 0.0f, -1000.0f, 1.0f),
	// 			Vector4( 1000.0f, 0.0f,  1000.0f, 1.0f),
	// 			Vector4(-1000.0f, 0.0f,  1000.0f, 1.0f),
	// 			Color4ub(0, 0, 0, 0)
	// 		);
	// 		m_primitiveRenderer->popDepthState();

	// 		m_primitiveRenderer->popView();
	// 		m_primitiveRenderer->end(0);
	// 		m_primitiveRenderer->render(m_renderView, 0);
	// 	}

	// 	m_renderView->endPass();
	// }

	// //if (m_postProcess)
	// //{
	// //	cl.mask = render::CfColor | render::CfDepth | render::CfStencil;
	// //	cl.colors[0] = Color4f(tmp[0], tmp[1], tmp[2], tmp[3]);
	// //	
	// //	m_renderView->begin(m_postTargetSet, 0, &cl);
	// //}

	// if (m_effectInstance)
	// {
	// 	m_context.deltaTime = deltaTime * m_timeScale;
	// 	m_context.soundPlayer = m_soundPlayer;

	// 	Transform effectTransform = Transform::identity();
	// 	if (m_moveEmitter)
	// 	{
	// 		Vector4 effectPosition(
	// 			std::sin(time) * 8.0f,
	// 			0.0f,
	// 			std::cos(time) * 8.0f,
	// 			1.0f
	// 		);
	// 		Vector4 effectDirection(
	// 			std::cos(time),
	// 			0.0f,
	// 			-std::sin(time),
	// 			0.0f
	// 		);
	// 		effectTransform = Transform(
	// 			effectPosition,
	// 			Quaternion(
	// 				Vector4(0.0f, 0.0f, 1.0f),
	// 				effectDirection
	// 			)
	// 		);
	// 	}

	// 	m_effectInstance->update(m_context, effectTransform, true);
	// 	m_effectInstance->synchronize();
	// 	m_effectInstance->render(
	// 		render::getParameterHandle(L"World_SimpleColor"),
	// 		m_pointRenderer,
	// 		m_meshRenderer,
	// 		m_trailRenderer,
	// 		Transform::identity(),
	// 		cameraPosition,
	// 		cameraPlane
	// 	);

	// 	Frustum viewFrustum;
	// 	viewFrustum.buildPerspective(80.0f * PI / 180.0f, aspect, 0.1f, 2000.0f);

	// 	world::WorldRenderView worldRenderView;
	// 	worldRenderView.setProjection(projectionTransform);
	// 	worldRenderView.setView(viewTransform, viewTransform);
	// 	worldRenderView.setViewSize(Vector2(float(sz.cx), float(sz.cy)));
	// 	worldRenderView.setCullFrustum(viewFrustum);
	// 	worldRenderView.setViewFrustum(viewFrustum);
	// 	worldRenderView.setTimes(time, deltaTime, 0.0f);

	// 	// world::Light globalLight;
	// 	// globalLight.type = world::LtDirectional;
	// 	// globalLight.position = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	// 	// globalLight.direction = Vector4(0.0f, 1.0f, -1.0f, 0.0f).normalized();
	// 	// globalLight.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	// 	// globalLight.range = Scalar(0.0f);
	// 	// worldRenderView.addLight(globalLight);

	// 	render::ProgramParameters visualProgramParams;
	// 	visualProgramParams.beginParameters(m_globalContext);
	// 	visualProgramParams.setFloatParameter(c_handleTime, time);
	// 	visualProgramParams.setMatrixParameter(c_handleView, viewTransform);
	// 	visualProgramParams.setMatrixParameter(c_handleProjection, projectionTransform);
	// 	visualProgramParams.endParameters(m_globalContext);

	// 	world::WorldRenderPassSimple defaultPass(
	// 		c_handleSimpleColor,
	// 		&visualProgramParams,
	// 		viewTransform
	// 	);

	// 	m_pointRenderer->flush(m_renderContext, defaultPass);
	// 	m_meshRenderer->flush(m_renderContext, defaultPass);
	// 	m_trailRenderer->flush(m_renderContext, defaultPass);

	// 	m_renderContext->merge(render::RpAll);
	// 	m_renderContext->render(m_renderView);
	// 	m_renderContext->flush();

	// 	m_globalContext->flush();

	// 	m_lastDeltaTime = deltaTime;
	// }

	// //if (m_postProcess)
	// //{
	// //	m_renderView->end();

	// //	render::ImageProcessStep::Instance::RenderParams params;
	// //	params.view = viewTransform;
	// //	params.viewToLight = Matrix44::identity();
	// //	params.projection = projectionTransform;
	// //	params.godRayDirection = Vector4(0.0f, 0.0f, -1.0f);
	// //	params.sliceCount = 0;
	// //	params.sliceIndex = 0;
	// //	params.sliceNearZ = 0.0f;
	// //	params.sliceFarZ = 0.0f;
	// //	params.shadowFarZ = 0.0f;
	// //	params.shadowMapBias = 0.0f;
	// //	params.deltaTime = deltaTime;
	// //	m_postProcess->render(
	// //		m_renderView,
	// //		m_postTargetSet->getColorTexture(0),
	// //		0,
	// //		0,
	// //		0,
	// //		0,
	// //		params
	// //	);
	// //}

	// if (!m_primitiveRenderer->begin(0, projectionTransform))
	// 	return;

	// m_primitiveRenderer->pushView(viewTransform);

	// for (int x = -10; x <= 10; ++x)
	// {
	// 	m_primitiveRenderer->drawLine(
	// 		Vector4(float(x), 0.0f, -10.0f, 1.0f),
	// 		Vector4(float(x), 0.0f, 10.0f, 1.0f),
	// 		(x == 0) ? 2.0f : 0.0f,
	// 		m_colorGrid
	// 	);
	// 	m_primitiveRenderer->drawLine(
	// 		Vector4(-10.0f, 0.0f, float(x), 1.0f),
	// 		Vector4(10.0f, 0.0f, float(x), 1.0f),
	// 		(x == 0) ? 2.0f : 0.0f,
	// 		m_colorGrid
	// 	);
	// }

	// // Draw emitter sources.
	// if (m_effectData && m_guideVisible)
	// {
	// 	const RefArray< EffectLayerData >& layers = m_effectData->getLayers();
	// 	for (RefArray< EffectLayerData >::const_iterator i = layers.begin(); i != layers.end(); ++i)
	// 	{
	// 		Ref< const EmitterData > emitterData = (*i)->getEmitter();
	// 		if (!emitterData)
	// 			continue;

	// 		Ref< const SourceData > sourceData = emitterData->getSource();
	// 		if (!sourceData)
	// 			continue;

	// 		std::map< const TypeInfo*, Ref< SourceRenderer > >::const_iterator j = m_sourceRenderers.find(&type_of(sourceData));
	// 		if (j != m_sourceRenderers.end())
	// 			j->second->render(m_primitiveRenderer, sourceData);
	// 	}
	// }

	// m_primitiveRenderer->end(0);
	// m_primitiveRenderer->render(m_renderView, 0);

	// m_renderView->endPass();
	// m_renderView->present();

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
