/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Math/Format.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Timer/Profiler.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Runtime/IAudioServer.h"
#include "Runtime/IEnvironment.h"
#include "Runtime/UpdateInfo.h"
#include "Runtime/Engine/Stage.h"
#include "Runtime/Engine/WorldLayer.h"
#include "Scene/Scene.h"
#include "Sound/Filters/SurroundEnvironment.h"
#include "Spray/Feedback/FeedbackManager.h"
#include "World/IWorldRenderer.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity.h"
#include "World/EntityData.h"
#include "World/IEntityBuilder.h"
#include "World/EntityEventManager.h"
#include "World/Entity/CameraComponent.h"
#include "World/Entity/GroupComponent.h"

namespace traktor::runtime
{
	namespace
	{

const Color4f c_clearColor(0.0f, 0.0f, 0.0f, 0.0f);
render::Handle s_handleFeedback(L"Feedback");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.WorldLayer", WorldLayer, Layer)

WorldLayer::WorldLayer(
	Stage* stage,
	const std::wstring& name,
	bool permitTransition,
	IEnvironment* environment,
	const resource::Proxy< scene::Scene >& scene
)
:	Layer(stage, name, permitTransition)
,	m_environment(environment)
,	m_scene(scene)
{
	// Create managment entities.
	m_rootGroup = new world::Entity();
	m_rootGroup->setComponent(new world::GroupComponent());

	m_dynamicEntities = new world::Entity();
	m_dynamicEntities->setComponent(new world::GroupComponent());

	// Get initial field of view.
	m_fieldOfView = m_environment->getSettings()->getProperty< float >(L"World.FieldOfView", 70.0f);

	// Register ourself for camera shake.
	spray::IFeedbackManager* feedbackManager = m_environment->getWorld()->getFeedbackManager();
	if (feedbackManager)
	{
		feedbackManager->addListener(spray::FbtCamera, this);
		feedbackManager->addListener(spray::FbtImageProcess, this);
	}
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

	m_environment = nullptr;
	m_rootGroup = nullptr;
	m_renderGroup = nullptr;
	m_cameraEntity = nullptr;
	m_listenerEntity = nullptr;

	if (m_scene)
	{
		m_scene->destroy();
		m_scene.clear();
	}

	safeDestroy(m_worldRenderer);
	safeDestroy(m_dynamicEntities);

	Layer::destroy();
}

void WorldLayer::transition(Layer* fromLayer)
{
	const bool permit = fromLayer->isTransitionPermitted() && isTransitionPermitted();
	if (!permit)
		return;

	WorldLayer* fromWorldLayer = mandatory_non_null_type_cast< WorldLayer* >(fromLayer);
	if (DeepHash(m_scene->getWorldRenderSettings()) == DeepHash(fromWorldLayer->m_scene->getWorldRenderSettings()))
	{
		m_worldRenderer = fromWorldLayer->m_worldRenderer;
		m_worldRenderView = fromWorldLayer->m_worldRenderView;

		fromWorldLayer->m_worldRenderer = nullptr;

		// Also need to ensure scene change doesn't reset world renderer.
		m_scene.consume();
	}
}

void WorldLayer::preUpdate(const UpdateInfo& info)
{
	T_PROFILER_SCOPE(L"WorldLayer pre-update");
	if (m_scene.changed())
	{
		// Scene has been successfully validated; drop existing world renderer if we've been flushed.
		m_worldRenderer = nullptr;
		m_scene.consume();

		// Get initial camera.
		m_cameraEntity = getEntity(L"Camera");
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
	const int32_t width = m_environment->getRender()->getWidth();
	const int32_t height = m_environment->getRender()->getHeight();

	// Update world view.
	m_worldRenderView.setPerspective(
		float(width),
		float(height),
		m_environment->getRender()->getAspectRatio(),
		deg2rad(m_fieldOfView),
		m_scene->getWorldRenderSettings()->viewNearZ,
		m_scene->getWorldRenderSettings()->viewFarZ
	);

	// Set projection from camera component.
	if (m_cameraEntity)
	{
		const world::CameraComponent* camera = m_cameraEntity->getComponent< world::CameraComponent >();
		if (camera)
		{
			if (camera->getProjection() == world::Projection::Orthographic)
			{
				m_worldRenderView.setOrthogonal(
					camera->getWidth(),
					camera->getHeight(),
					m_scene->getWorldRenderSettings()->viewNearZ,
					m_scene->getWorldRenderSettings()->viewFarZ
				);
			}
			else // Projection::Perspective
			{
				m_worldRenderView.setPerspective(
					float(width),
					float(height),
					m_environment->getRender()->getAspectRatio(),
					camera->getFieldOfView(),
					m_scene->getWorldRenderSettings()->viewNearZ,
					m_scene->getWorldRenderSettings()->viewFarZ
				);
			}
		}
	}

	// Update sound listener transform.
	if (m_environment->getAudio())
	{
		sound::SurroundEnvironment* surroundEnvironment = m_environment->getAudio()->getSurroundEnvironment();
		if (surroundEnvironment && (m_listenerEntity || m_cameraEntity))
		{
			Transform listenerTransform;

			if (m_listenerEntity)
				listenerTransform = m_listenerEntity->getTransform();
			else
				listenerTransform = m_cameraEntity->getTransform();

			surroundEnvironment->setListenerTransform(listenerTransform);
		}
	}
}

void WorldLayer::update(const UpdateInfo& info)
{
	T_PROFILER_SCOPE(L"WorldLayer update");
	if (!m_worldRenderer)
		return;

	// Update camera transform from entity.
	if (m_cameraEntity)
	{
		Transform cameraTransform = m_cameraEntity->getTransform();
		m_cameraTransform.step();
		m_cameraTransform.set(cameraTransform);
	}

	// Update scene controller.
	if (m_controllerEnable)
	{
		if (m_controllerTime < 0.0f)
			m_controllerTime = info.getSimulationTime();

		world::UpdateParams up;
		up.contextObject = getStage();
		up.totalTime = info.getSimulationTime() - m_controllerTime;
		up.deltaTime = info.getSimulationDeltaTime();
		up.alternateTime = m_alternateTime;

		m_scene->updateController(up);
	}

	{
		world::UpdateParams up;
		up.contextObject = getStage();
		up.totalTime = info.getSimulationTime();
		up.deltaTime = info.getSimulationDeltaTime();
		up.alternateTime = m_alternateTime;

		// Update all entities.
		m_scene->updateEntity(up);
		m_dynamicEntities->update(up);

		// Update entity events.
		world::EntityEventManager* eventManager = m_environment->getWorld()->getEntityEventManager();
		if (eventManager)
			eventManager->update(up);
	}

	// In case not explicitly set we update the alternative time also.
	m_alternateTime += info.getSimulationDeltaTime();
}

void WorldLayer::preSetup(const UpdateInfo& info)
{
	T_PROFILER_SCOPE(L"WorldLayer pre-setup");
	if (!m_worldRenderer || !m_scene)
		return;

	// Grab interpolated camera transform.
	if (m_cameraEntity)
	{
		m_worldRenderView.setView(
			m_worldRenderView.getView(),
			(m_cameraTransform.get(info.getInterval()) * m_cameraOffset).inverse().toMatrix44()
		);
	}

	m_worldRenderView.setTimes(
		info.getStateTime(),
		info.getFrameDeltaTime(),
		info.getInterval()
	);
}

void WorldLayer::setup(const UpdateInfo& info, render::RenderGraph& renderGraph)
{
	T_PROFILER_SCOPE(L"WorldLayer setup");
	if (!m_worldRenderer || !m_scene)
		return;

	// Build a root entity by gathering entities from containers.
	auto group = m_rootGroup->getComponent< world::GroupComponent >();
	group->removeAllEntities();

	world::EntityEventManager* eventManager = m_environment->getWorld()->getEntityEventManager();
	if (eventManager)
		eventManager->gather([&](world::Entity* entity) { group->addEntity(entity); });

	group->addEntity(m_scene->getRootEntity());
	group->addEntity(m_dynamicEntities);

	// Add render passes with world renderer.
	m_worldRenderer->setup(
		m_worldRenderView,
		m_rootGroup,
		renderGraph,
		0
	);		
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
	preUpdate(info);
}

void WorldLayer::suspend()
{
#if defined(__ANDROID__) || defined(__IOS__)
	// Destroy previous world renderer; do this
	// to save memory which is easily re-constructed at resume.
	safeDestroy(m_worldRenderer);
#endif
}

void WorldLayer::resume()
{
}

scene::Scene* WorldLayer::getScene() const
{
	return m_scene;
}

world::IWorldRenderer* WorldLayer::getWorldRenderer() const
{
	return m_worldRenderer;
}

world::Entity* WorldLayer::getEntity(const std::wstring& name) const
{
	return getEntity(name, 0);
}

world::Entity* WorldLayer::getEntity(const std::wstring& name, int32_t index) const
{
	{
		auto group = m_scene->getRootEntity()->getComponent< world::GroupComponent >();
		if (group)
		{
			auto entity = group->getEntity(name, index);
			if (entity)
				return entity;
		}
	}
	{
		auto group = m_dynamicEntities->getComponent< world::GroupComponent >();
		if (group)
		{
			auto entity = group->getEntity(name, index);
			if (entity)
				return entity;
		}
	}
	return nullptr;
}

RefArray< world::Entity > WorldLayer::getEntities(const std::wstring& name) const
{
	RefArray< world::Entity > entities;
	{
		auto group = m_scene->getRootEntity()->getComponent< world::GroupComponent >();
		if (group)
		{
			auto e = group->getEntities(name);
			entities.insert(entities.end(), e.begin(), e.end());
		}
	}
	{
		auto group = m_dynamicEntities->getComponent< world::GroupComponent >();
		if (group)
		{
			auto e = group->getEntities(name);
			entities.insert(entities.end(), e.begin(), e.end());
		}
	}
	return entities;
}

void WorldLayer::addEntity(world::Entity* entity)
{
	if (m_dynamicEntities)
		m_dynamicEntities->getComponent< world::GroupComponent >()->addEntity(entity);
}

void WorldLayer::removeEntity(world::Entity* entity)
{
	if (m_dynamicEntities)
		m_dynamicEntities->getComponent< world::GroupComponent >()->removeEntity(entity);
}

bool WorldLayer::isEntityAdded(const world::Entity* entity) const
{
	if (m_dynamicEntities)
	{
		const auto& entities = m_dynamicEntities->getComponent< world::GroupComponent >()->getEntities();
		return std::find(entities.begin(), entities.end(), entity) != entities.end();
	}
	else
		return false;
}

void WorldLayer::setControllerEnable(bool controllerEnable)
{
	m_controllerEnable = controllerEnable;
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

	const Vector4 t = lerp(viewFrustum.corners[0], viewFrustum.corners[1], Scalar(screenPosition.x * 0.5f + 0.5f));
	const Vector4 b = lerp(viewFrustum.corners[3], viewFrustum.corners[2], Scalar(screenPosition.x * 0.5f + 0.5f));
	const Vector4 p = lerp(b, t, Scalar(screenPosition.y * 0.5f + 0.5f));

	outViewRay.origin = Vector4::origo();
	outViewRay.direction = p.xyz0().normalized();
	return true;
}

bool WorldLayer::screenToWorld(const Vector2& screenPosition, Ray3& outWorldRay) const
{
	Ray3 viewRay;
	if (!screenToView(screenPosition, viewRay))
		return false;

	const Matrix44 viewInverse = m_worldRenderView.getView().inverse();
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
		T_ASSERT(count >= 4);
		m_cameraOffset = Transform(
			Vector4(values[0], values[1], values[2]) * Scalar(m_feedbackScale),
			Quaternion::fromEulerAngles(0.0f, 0.0f, values[3] * m_feedbackScale)
		);
	}
	else if (type == spray::FbtImageProcess)
	{
		T_ASSERT(count >= 4);

		// \fixme
		// render::ImageProcess* postProcess = m_worldRenderer->getVisualImageProcess();
		// if (postProcess)
		// 	postProcess->setVectorParameter(s_handleFeedback, Vector4::loadUnaligned(values));
	}
}

}
