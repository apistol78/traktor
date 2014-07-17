#include "Amalgam/FrameProfiler.h"
#include "Amalgam/IAudioServer.h"
#include "Amalgam/IEnvironment.h"
#include "Amalgam/IUpdateInfo.h"
#include "Amalgam/Engine/WorldLayer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Scene/Scene.h"
#include "Sound/Filters/SurroundEnvironment.h"
#include "Spray/Feedback/FeedbackManager.h"
#include "World/IWorldRenderer.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity.h"
#include "World/EntityData.h"
#include "World/IEntityBuilder.h"
#include "World/IEntitySchema.h"
#include "World/IEntityEventManager.h"
#include "World/Entity/GroupEntity.h"
#include "World/Entity/NullEntity.h"
#include "World/Entity/TransientEntity.h"
#include "World/PostProcess/PostProcess.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

const Color4f c_clearColor(0.0f, 0.0f, 0.0f, 0.0f);
render::handle_t s_handleFeedback;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.WorldLayer", WorldLayer, Layer)

WorldLayer::WorldLayer(
	Stage* stage,
	const std::wstring& name,
	amalgam::IEnvironment* environment,
	const resource::Proxy< scene::Scene >& scene,
	const std::map< std::wstring, resource::Proxy< world::EntityData > >& entities
)
:	Layer(stage, name)
,	m_environment(environment)
,	m_scene(scene)
,	m_entities(entities)
,	m_dynamicEntities(new world::GroupEntity())
,	m_camera(L"Camera")
,	m_alternateTime(0.0f)
,	m_deltaTime(0.0f)
,	m_controllerTime(-1.0f)
,	m_fieldOfView(70.0f)
,	m_feedbackScale(1.0f)
,	m_controllerEnable(true)
{
	// Get initial field of view.
	m_fieldOfView = m_environment->getSettings()->getProperty< PropertyFloat >(L"World.FieldOfView", 70.0f);

	// Register ourself for camera shake.
	spray::IFeedbackManager* feedbackManager = m_environment->getWorld()->getFeedbackManager();
	if (feedbackManager)
	{
		feedbackManager->addListener(spray::FbtCamera, this);
		feedbackManager->addListener(spray::FbtPostProcess, this);
	}

	// Get parameter handles.
	s_handleFeedback = render::getParameterHandle(L"Feedback");
}

WorldLayer::~WorldLayer()
{
	destroy();
}

void WorldLayer::destroy()
{
	// Remove ourself from feedback manager.
	spray::IFeedbackManager* feedbackManager = m_environment->getWorld()->getFeedbackManager();
	if (feedbackManager)
	{
		feedbackManager->removeListener(spray::FbtPostProcess, this);
		feedbackManager->removeListener(spray::FbtCamera, this);
	}

	m_environment = 0;

	if (m_scene)
	{
		m_scene->destroy();
		m_scene.clear();
	}

	m_entities.clear();
	safeDestroy(m_worldRenderer);
	safeDestroy(m_renderGroup);
	safeDestroy(m_dynamicEntities);
}

void WorldLayer::transition(Layer* fromLayer)
{
}

void WorldLayer::prepare()
{
	if (m_scene.changed())
	{
		// If render group already exist then ensure it doesn't contain anything
		// before begin re-created as it will otherwise destroy it's children.
		if (m_renderGroup)
		{
			m_renderGroup->removeAllEntities();
			m_renderGroup = 0;
		}

		// Create render entity group; contain scene root as well as dynamic entities.
		m_renderGroup = new world::GroupEntity();
		m_renderGroup->addEntity(m_scene->getRootEntity());
		m_renderGroup->addEntity(m_dynamicEntities);

		// Scene has been successfully validated; drop existing world renderer if we've been flushed.
		m_worldRenderer = 0;
		m_scene.consume();
	}

	// Re-create world renderer.
	if (!m_worldRenderer)
	{
		createWorldRenderer();
		if (!m_worldRenderer)
			return;
	}
}

void WorldLayer::update(amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info)
{
	if (!m_worldRenderer)
		return;

	info.getProfiler()->beginScope(FptWorldLayer);

	// Update scene controller.
	if (m_controllerEnable)
	{
		if (m_controllerTime < 0.0f)
			m_controllerTime = info.getSimulationTime();

		world::UpdateParams up;
		up.totalTime = info.getSimulationTime() - m_controllerTime;
		up.deltaTime = info.getSimulationDeltaTime();
		up.alternateTime = m_alternateTime;

		m_scene->update(up, true, false);
	}

	{
		world::UpdateParams up;
		up.totalTime = info.getSimulationTime();
		up.deltaTime = info.getSimulationDeltaTime();
		up.alternateTime = m_alternateTime;

		// Update all entities; calling manually because we have exclusive control
		// of dynamic entities and an explicit render root group.
		m_renderGroup->update(up);

		// Update entity events.
		world::IEntityEventManager* eventManager = m_environment->getWorld()->getEntityEventManager();
		if (eventManager)
		{
			info.getProfiler()->beginScope(FptWorldLayerEvents);
			eventManager->update(up);
			info.getProfiler()->endScope();
		}
	}

	// In case not explicitly set we update the alternative time also.
	m_alternateTime += info.getSimulationDeltaTime();

	info.getProfiler()->endScope();
}

void WorldLayer::build(const amalgam::IUpdateInfo& info, uint32_t frame)
{
	if (!m_worldRenderer)
		return;

	// Get camera entity and extract view transform.
	world::NullEntity* cameraEntity = m_scene->getEntitySchema()->getEntity< world::NullEntity >(m_camera);
	if (cameraEntity)
	{
		Transform view = cameraEntity->getTransform(info.getInterval()) * m_cameraOffset;
		m_worldRenderView.setView(view.inverse().toMatrix44());

		// Also update sound listener transform based on camera.
		sound::SurroundEnvironment* surroundEnvironment = m_environment->getAudio()->getSurroundEnvironment();
		if (surroundEnvironment)
			surroundEnvironment->setListenerTransform(view);
	}

	// Build frame through world renderer.
	m_worldRenderView.setTimes(
		info.getStateTime(),
		info.getFrameDeltaTime(),
		info.getInterval()
	);
	if (m_worldRenderer->beginBuild())
	{
		m_worldRenderer->build(m_renderGroup);

		world::IEntityEventManager* eventManager = m_environment->getWorld()->getEntityEventManager();
		if (eventManager)
			eventManager->build(m_worldRenderer);

		m_worldRenderer->endBuild(m_worldRenderView, frame);
	}

	m_deltaTime = info.getFrameDeltaTime();
}

void WorldLayer::render(render::EyeType eye, uint32_t frame)
{
	if (!m_scene || !m_worldRenderer)
		return;

	render::IRenderView* renderView = m_environment->getRender()->getRenderView();
	T_ASSERT (renderView);

	if (m_worldRenderer->beginRender(frame, eye, c_clearColor))
	{
		// Bind per-scene post processing parameters.
		world::PostProcess* postProcess = m_worldRenderer->getVisualPostProcess();
		if (postProcess)
		{
			for (SmallMap< render::handle_t, resource::Proxy< render::ITexture > >::const_iterator i = m_scene->getPostProcessParams().begin(); i != m_scene->getPostProcessParams().end(); ++i)
				postProcess->setTextureParameter(i->first, i->second);
		}

		// Render world.
		m_worldRenderer->render(
			world::WrfDepthMap | world::WrfNormalMap | world::WrfShadowMap | world::WrfLightMap | world::WrfVisualOpaque | world::WrfVisualAlphaBlend,
			frame,
			eye
		);

		m_worldRenderer->endRender(frame, eye, m_deltaTime);
	}
}

void WorldLayer::preReconfigured()
{
	// Destroy previous world renderer; do this
	// before re-configuration of servers as world
	// renderer might have heavy resources already created
	// such as render targets and textures.
	safeDestroy(m_worldRenderer);
}

void WorldLayer::postReconfigured()
{
}

Ref< world::EntityData > WorldLayer::getEntityData(const std::wstring& name) const
{
	std::map< std::wstring, resource::Proxy< world::EntityData > >::const_iterator i = m_entities.find(name);
	if (i != m_entities.end())
		return DeepClone(i->second.getResource()).create< world::EntityData >();
	else
		return 0;
}

world::Entity* WorldLayer::getEntity(const std::wstring& name) const
{
	return m_scene->getEntitySchema()->getEntity(name);
}

RefArray< world::Entity > WorldLayer::getEntities(const std::wstring& name) const
{
	RefArray< world::Entity > entities;
	m_scene->getEntitySchema()->getEntities(name, entities);
	return entities;
}

RefArray< world::Entity > WorldLayer::getEntitiesOf(const TypeInfo& entityType) const
{
	RefArray< world::Entity > entities;
	m_scene->getEntitySchema()->getEntities(entityType, entities);
	return entities;
}

Ref< world::Entity > WorldLayer::createEntity(const std::wstring& name, world::IEntitySchema* entitySchema)
{
	std::map< std::wstring, resource::Proxy< world::EntityData > >::iterator i = m_entities.find(name);
	if (i == m_entities.end())
		return 0;

	const world::IEntityBuilder* entityBuilder = m_environment->getWorld()->getEntityBuilder();
	T_ASSERT (entityBuilder);

	return entityBuilder->create(i->second);
}

int32_t WorldLayer::getEntityIndex(const world::Entity* entity) const
{
	RefArray< world::Entity > entities;
	m_scene->getEntitySchema()->getEntities(entities);

	RefArray< world::Entity >::const_iterator i = std::find(entities.begin(), entities.end(), entity);
	if (i == entities.end())
		return -1;

	return std::distance< RefArray< world::Entity >::const_iterator >(entities.begin(), i);
}

int32_t WorldLayer::getEntityIndexOf(const world::Entity* entity) const
{
	RefArray< world::Entity > entities;
	m_scene->getEntitySchema()->getEntities(type_of(entity), entities);

	RefArray< world::Entity >::const_iterator i = std::find(entities.begin(), entities.end(), entity);
	if (i == entities.end())
		return -1;

	return std::distance< RefArray< world::Entity >::const_iterator >(entities.begin(), i);
}

world::Entity* WorldLayer::getEntityByIndex(int32_t index) const
{
	return m_scene->getEntitySchema()->getEntity(index);
}

world::Entity* WorldLayer::getEntityOf(const TypeInfo& entityType, int32_t index) const
{
	return m_scene->getEntitySchema()->getEntity(entityType, index);
}

void WorldLayer::addEntity(world::Entity* entity)
{
	if (m_dynamicEntities)
		m_dynamicEntities->addEntity(entity);
}

void WorldLayer::addTransientEntity(world::Entity* entity, float duration)
{
	if (m_dynamicEntities)
		m_dynamicEntities->addEntity(new world::TransientEntity(m_dynamicEntities, entity, duration));
}

void WorldLayer::removeEntity(world::Entity* entity)
{
	if (m_dynamicEntities)
		m_dynamicEntities->removeEntity(entity);
}

world::IEntitySchema* WorldLayer::getEntitySchema() const
{
	return m_scene->getEntitySchema();
}

void WorldLayer::setControllerEnable(bool controllerEnable)
{
	m_controllerEnable = controllerEnable;
}

world::PostProcess* WorldLayer::getPostProcess() const
{
	return m_worldRenderer->getVisualPostProcess();
}

void WorldLayer::resetController()
{
	m_controllerTime = -1.0f;
}

const Frustum& WorldLayer::getViewFrustum() const
{
	return m_worldRenderView.getViewFrustum();
}

bool WorldLayer::worldToView(const Vector4& worldPosition, Vector4& outViewPosition) const
{
	outViewPosition = m_worldRenderView.getView() * worldPosition;
	return true;
}

bool WorldLayer::viewToWorld(const Vector4& viewPosition, Vector4& outWorldPosition) const
{
	outWorldPosition = m_worldRenderView.getView().inverse() * viewPosition;
	return true;
}

bool WorldLayer::worldToScreen(const Vector4& worldPosition, Vector2& outScreenPosition) const
{
	Vector4 viewPosition = m_worldRenderView.getView() * worldPosition;
	return viewToScreen(viewPosition, outScreenPosition);
}

bool WorldLayer::viewToScreen(const Vector4& viewPosition, Vector2& outScreenPosition) const
{
	Vector4 clipPosition = m_worldRenderView.getProjection() * viewPosition;
	if (clipPosition.w() <= 0.0f)
		return false;
	clipPosition /= clipPosition.w();
	outScreenPosition = Vector2(clipPosition.x(), clipPosition.y());
	return true;
}

void WorldLayer::setFieldOfView(float fieldOfView)
{
	if (fieldOfView != m_fieldOfView)
	{
		render::IRenderView* renderView = m_environment->getRender()->getRenderView();
		T_ASSERT (renderView);

		// Get render view dimensions.
		int32_t width = renderView->getWidth();
		int32_t height = renderView->getHeight();

		// Create world view.
		world::WorldViewPerspective worldViewPort;
		worldViewPort.width = width;
		worldViewPort.height = height;
		worldViewPort.aspect = m_environment->getRender()->getAspectRatio();
		worldViewPort.fov = deg2rad(fieldOfView);
		m_worldRenderer->createRenderView(worldViewPort, m_worldRenderView);

		// Save field of view value as we must be able to re-create
		// world view if view port dimensions change.
		m_fieldOfView = fieldOfView;
	}
}

float WorldLayer::getFieldOfView() const
{
	return m_fieldOfView;
}

void WorldLayer::setAlternateTime(float alternateTime)
{
	m_alternateTime = alternateTime;
}

float WorldLayer::getAlternateTime() const
{
	return m_alternateTime;
}

void WorldLayer::setFeedbackScale(float feedbackScale)
{
	m_feedbackScale = feedbackScale;
}

float WorldLayer::getFeedbackScale() const
{
	return m_feedbackScale;
}

void WorldLayer::setCamera(const std::wstring& camera)
{
	m_camera = camera;
}

const std::wstring& WorldLayer::getCamera() const
{
	return m_camera;
}

void WorldLayer::feedbackValues(spray::FeedbackType type, const float* values, int32_t count)
{
	if (type == spray::FbtCamera)
	{
		T_ASSERT (count >= 4);
		m_cameraOffset = Transform(
			Vector4(values[0], values[1], values[2]) * Scalar(m_feedbackScale),
			Quaternion::fromEulerAngles(0.0f, 0.0f, values[3] * m_feedbackScale)
		);
	}
	else if (type == spray::FbtPostProcess)
	{
		T_ASSERT (count >= 4);
		world::PostProcess* postProcess = m_worldRenderer->getVisualPostProcess();
		if (postProcess)
			postProcess->setVectorParameter(s_handleFeedback, Vector4::loadUnaligned(values));
	}
}

void WorldLayer::createWorldRenderer()
{
	render::IRenderView* renderView = m_environment->getRender()->getRenderView();

	// Get render view dimensions.
	int32_t width = renderView->getWidth();
	int32_t height = renderView->getHeight();

	// Get post process quality from settings.
	int32_t postProcessQuality = m_environment->getSettings()->getProperty< PropertyInteger >(L"World.PostProcessQuality", world::QuHigh);

	// Create world renderer.
	m_worldRenderer = m_environment->getWorld()->createWorldRenderer(
		m_scene->getWorldRenderSettings(),
		m_scene->getPostProcessSettings((world::Quality)postProcessQuality)
	);
	if (!m_worldRenderer)
	{
		log::error << L"Unable to create world renderer; world layer disabled" << Endl;
		return;
	}

	// Create world render view.
	world::WorldViewPerspective worldViewPort;
	worldViewPort.width = width;
	worldViewPort.height = height;
	worldViewPort.aspect = m_environment->getRender()->getAspectRatio();
	worldViewPort.fov = deg2rad(m_fieldOfView);
	m_worldRenderer->createRenderView(worldViewPort, m_worldRenderView);
}

	}
}
