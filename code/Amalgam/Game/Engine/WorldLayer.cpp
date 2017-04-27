/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/Game/IAudioServer.h"
#include "Amalgam/Game/IEnvironment.h"
#include "Amalgam/Game/UpdateInfo.h"
#include "Amalgam/Game/Engine/WorldLayer.h"
#include "Core/Log/Log.h"
#include "Core/Math/Format.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Timer/Profiler.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/ImageProcess/ImageProcess.h"
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
#include "World/Entity/CameraComponent.h"
#include "World/Entity/ComponentEntity.h"
#include "World/Entity/GroupEntity.h"

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
	bool permitTransition,
	IEnvironment* environment,
	const resource::Proxy< scene::Scene >& scene,
	const std::map< std::wstring, resource::Proxy< world::EntityData > >& entities
)
:	Layer(stage, name, permitTransition)
,	m_environment(environment)
,	m_scene(scene)
,	m_entities(entities)
,	m_dynamicEntities(new world::GroupEntity())
,	m_alternateTime(0.0f)
,	m_deltaTime(0.0f)
,	m_controllerTime(-1.0f)
,	m_fieldOfView(70.0f)
,	m_feedbackScale(1.0f)
,	m_controllerEnable(true)
{
	// Get initial field of view.
	m_fieldOfView = m_environment->getSettings()->getProperty< float >(L"World.FieldOfView", 70.0f);

	// Register ourself for camera shake.
	spray::IFeedbackManager* feedbackManager = m_environment->getWorld()->getFeedbackManager();
	if (feedbackManager)
	{
		feedbackManager->addListener(spray::FbtCamera, this);
		feedbackManager->addListener(spray::FbtImageProcess, this);
	}

	// Get parameter handles.
	s_handleFeedback = render::getParameterHandle(L"Feedback");
}

void WorldLayer::destroy()
{
	// Remove ourself from feedback manager.
	spray::IFeedbackManager* feedbackManager = m_environment->getWorld()->getFeedbackManager();
	if (feedbackManager)
	{
		feedbackManager->removeListener(spray::FbtImageProcess, this);
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

	Layer::destroy();
}

void WorldLayer::transition(Layer* fromLayer)
{
	bool permit = fromLayer->isTransitionPermitted() && isTransitionPermitted();
	if (!permit)
		return;

	WorldLayer* fromWorldLayer = checked_type_cast< WorldLayer*, false >(fromLayer);
	if (DeepHash(m_scene->getWorldRenderSettings()) == DeepHash(fromWorldLayer->m_scene->getWorldRenderSettings()))
	{
		m_worldRenderer = fromWorldLayer->m_worldRenderer;
		m_worldRenderView = fromWorldLayer->m_worldRenderView;

		fromWorldLayer->m_worldRenderer = 0;

		// Create render entity group; contain scene root as well as dynamic entities.
		m_renderGroup = new world::GroupEntity();
		m_renderGroup->addEntity(m_scene->getRootEntity());
		m_renderGroup->addEntity(m_dynamicEntities);

		// Also need to ensure scene change doesn't reset world renderer.
		m_scene.consume();
	}
}

void WorldLayer::prepare(const UpdateInfo& info)
{
	T_PROFILER_SCOPE(L"WorldLayer prepare");
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

		// Get initial camera.
		m_cameraEntity = m_scene->getEntitySchema()->getEntity(L"Camera");
		m_listenerEntity = m_cameraEntity;
	}

	// Re-create world renderer.
	if (!m_worldRenderer)
	{
		m_worldRenderer = m_environment->getWorld()->createWorldRenderer(m_scene->getWorldRenderSettings());
		if (!m_worldRenderer)
			return;
	}

	// Get render view dimensions.
	int32_t width = m_environment->getRender()->getWidth();
	int32_t height = m_environment->getRender()->getHeight();

	// Update world view.
	m_worldRenderView.setPerspective(
		width,
		height,
		m_environment->getRender()->getAspectRatio(),
		deg2rad(m_fieldOfView),
		m_scene->getWorldRenderSettings()->viewNearZ,
		m_scene->getWorldRenderSettings()->viewFarZ
	);

	if (const world::ComponentEntity* cameraEntity = dynamic_type_cast< const world::ComponentEntity* >(m_cameraEntity))
	{
		const world::CameraComponent* camera = cameraEntity->getComponent< world::CameraComponent >();
		if (camera)
		{
			if (camera->getCameraType() == world::CtOrthographic)
			{
				m_worldRenderView.setOrthogonal(
					camera->getWidth(),
					camera->getHeight(),
					m_scene->getWorldRenderSettings()->viewNearZ,
					m_scene->getWorldRenderSettings()->viewFarZ
				);
			}
			else // CtPerspective
			{
				m_worldRenderView.setPerspective(
					width,
					height,
					m_environment->getRender()->getAspectRatio(),
					camera->getFieldOfView(),
					m_scene->getWorldRenderSettings()->viewNearZ,
					m_scene->getWorldRenderSettings()->viewFarZ
				);
			}
		}
	}

	if (m_cameraEntity)
		m_worldRenderView.setView(
			m_worldRenderView.getView(),
			(m_cameraTransform.get(info.getInterval()) * m_cameraOffset).inverse().toMatrix44()
		);

	m_worldRenderView.setTimes(
		info.getStateTime(),
		info.getFrameDeltaTime(),
		info.getInterval()
	);

	// Update sound listener transform.
	if (m_environment->getAudio())
	{
		sound::SurroundEnvironment* surroundEnvironment = m_environment->getAudio()->getSurroundEnvironment();
		if (surroundEnvironment && (m_listenerEntity || m_cameraEntity))
		{
			Transform listenerTransform;

			if (m_listenerEntity)
				m_listenerEntity->getTransform(listenerTransform);
			else
				m_cameraEntity->getTransform(listenerTransform);

			surroundEnvironment->setListenerTransform(listenerTransform);
		}
	}
}

void WorldLayer::update(const UpdateInfo& info)
{
	T_PROFILER_SCOPE(L"WorldLayer update");
	if (!m_worldRenderer)
		return;

	// Update camera transform.
	if (m_cameraEntity)
	{
		Transform cameraTransform;
		m_cameraEntity->getTransform(cameraTransform);
		m_cameraTransform.step();
		m_cameraTransform.set(cameraTransform);
	}

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
			eventManager->update(up);
	}

	// In case not explicitly set we update the alternative time also.
	m_alternateTime += info.getSimulationDeltaTime();
}

void WorldLayer::build(const UpdateInfo& info, uint32_t frame)
{
	T_PROFILER_SCOPE(L"WorldLayer build");
	if (!m_worldRenderer || !m_scene)
		return;

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
	T_PROFILER_SCOPE(L"WorldLayer render");
	if (!m_worldRenderer || !m_scene)
		return;

	if (m_worldRenderer->beginRender(frame, eye, c_clearColor))
	{
		// Bind per-scene post processing parameters.
		render::ImageProcess* postProcess = m_worldRenderer->getVisualImageProcess();
		if (postProcess)
		{
			for (SmallMap< render::handle_t, resource::Proxy< render::ITexture > >::const_iterator i = m_scene->getImageProcessParams().begin(); i != m_scene->getImageProcessParams().end(); ++i)
				postProcess->setTextureParameter(i->first, i->second);
		}

		// Render world.
		m_worldRenderer->render(frame, eye);
		m_worldRenderer->endRender(frame, eye, m_deltaTime);
	}
}

void WorldLayer::flush()
{
	// World renderer doesn't have a specific flush path; thus
	// we teer down the world renderer completely.
	safeDestroy(m_worldRenderer);
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
	// Issue prepare here as we want the world renderer
	// to be created during reconfiguration has the render lock.
	UpdateInfo info;
	prepare(info);
}

void WorldLayer::suspend()
{
#if defined(__IOS__)
	// Destroy previous world renderer; do this
	// to save memory which is easily re-constructed at resume.
	safeDestroy(m_worldRenderer);
#endif
}

void WorldLayer::resume()
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

world::Entity* WorldLayer::getEntity(const std::wstring& name, int32_t index) const
{
	return m_scene->getEntitySchema()->getEntity(name, index);
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
	T_PROFILER_SCOPE(L"WorldLayer createEntity");

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

void WorldLayer::removeEntity(world::Entity* entity)
{
	if (m_dynamicEntities)
		m_dynamicEntities->removeEntity(entity);
}

bool WorldLayer::isEntityAdded(const world::Entity* entity) const
{
	if (m_dynamicEntities)
	{
		const RefArray< world::Entity >& entities = m_dynamicEntities->getEntities();
		return std::find(entities.begin(), entities.end(), entity) != entities.end();
	}
	else
		return false;
}

world::IEntitySchema* WorldLayer::getEntitySchema() const
{
	return m_scene->getEntitySchema();
}

void WorldLayer::setControllerEnable(bool controllerEnable)
{
	m_controllerEnable = controllerEnable;
}

render::ImageProcess* WorldLayer::getImageProcess() const
{
	return m_worldRenderer->getVisualImageProcess();
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

bool WorldLayer::screenToView(const Vector2& screenPosition, Ray3& outViewRay) const
{
	const Frustum& viewFrustum = m_worldRenderView.getViewFrustum();

	Vector4 t = lerp(viewFrustum.corners[0], viewFrustum.corners[1], Scalar(screenPosition.x * 0.5f + 0.5f));
	Vector4 b = lerp(viewFrustum.corners[3], viewFrustum.corners[2], Scalar(screenPosition.x * 0.5f + 0.5f));
	Vector4 p = lerp(b, t, Scalar(screenPosition.y * 0.5f + 0.5f));

	outViewRay.origin = Vector4::origo();
	outViewRay.direction = p.xyz0().normalized();
	return true;
}

bool WorldLayer::screenToWorld(const Vector2& screenPosition, Ray3& outWorldRay) const
{
	Ray3 viewRay;
	if (!screenToView(screenPosition, viewRay))
		return false;

	Matrix44 viewInverse = m_worldRenderView.getView().inverse();
	outWorldRay.origin = viewInverse * viewRay.origin;
	outWorldRay.direction = viewInverse * viewRay.direction;
	return true;
}

void WorldLayer::setFieldOfView(float fieldOfView)
{
	m_fieldOfView = fieldOfView;
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

void WorldLayer::setCamera(const world::Entity* cameraEntity)
{
	m_cameraEntity = cameraEntity;
}

const world::Entity* WorldLayer::getCamera() const
{
	return m_cameraEntity;
}

void WorldLayer::setListener(const world::Entity* listenerEntity)
{
	m_listenerEntity = listenerEntity;
}

const world::Entity* WorldLayer::getListener() const
{
	return m_listenerEntity;
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
	else if (type == spray::FbtImageProcess)
	{
		T_ASSERT (count >= 4);
		render::ImageProcess* postProcess = m_worldRenderer->getVisualImageProcess();
		if (postProcess)
			postProcess->setVectorParameter(s_handleFeedback, Vector4::loadUnaligned(values));
	}
}

	}
}
