/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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
#include "Resource/IResourceManager.h"
#include "Runtime/IAudioServer.h"
#include "Runtime/IEnvironment.h"
#include "Runtime/UpdateInfo.h"
#include "Runtime/Engine/Stage.h"
#include "Runtime/Engine/WorldLayer.h"
#include "Scene/Scene.h"
#include "World/IWorldRenderer.h"
#include "World/World.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity.h"
#include "World/EntityBuilder.h"
#include "World/EntityData.h"
#include "World/Entity/CameraComponent.h"
#include "World/Entity/GroupComponent.h"
#include "World/Entity/PersistentIdComponent.h"

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
	// Get initial field of view.
	m_fieldOfView = m_environment->getSettings()->getProperty< float >(L"World.FieldOfView", 70.0f);
}

void WorldLayer::destroy()
{
	m_environment = nullptr;
	m_cameraEntity = nullptr;

	if (m_scene)
	{
		m_scene->destroy();
		m_scene.clear();
	}

	safeDestroy(m_worldRenderer);

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
		m_cameraEntity = m_scene->getWorld()->getEntity(L"Camera0");
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
}

void WorldLayer::update(const UpdateInfo& info)
{
	T_PROFILER_SCOPE(L"WorldLayer update");
	if (!m_worldRenderer)
		return;

	// Update scene controller.
	//if (m_controllerEnable)
	//{
	//	if (m_controllerTime < 0.0f)
	//		m_controllerTime = info.getSimulationTime();

	//	world::UpdateParams up;
	//	up.contextObject = getStage();
	//	up.totalTime = info.getSimulationTime() - m_controllerTime;
	//	up.alternateTime = m_alternateTime;
	//	up.deltaTime = info.getSimulationDeltaTime();

	//	m_scene->updateController(up);
	//}

	{
		world::UpdateParams up;
		up.contextObject = getStage();
		up.totalTime = info.getSimulationTime();
		up.alternateTime = m_alternateTime;
		up.deltaTime = info.getSimulationDeltaTime();

		// Update all entities.
		m_scene->update(up);
	}

	// In case not explicitly set we update the alternative time also.
	m_alternateTime += info.getSimulationDeltaTime();
}

void WorldLayer::postUpdate(const UpdateInfo& info)
{
	T_PROFILER_SCOPE(L"WorldLayer post-update");
	if (!m_worldRenderer || !m_scene)
		return;

	// Update camera transform from entity.
	if (m_cameraEntity)
	{
		const Transform cameraTransform = m_cameraEntity->getTransform();
		m_cameraTransform.step();
		m_cameraTransform.set(cameraTransform);
	}
}

void WorldLayer::preSetup(const UpdateInfo& info)
{
	T_PROFILER_SCOPE(L"WorldLayer pre-setup");
	if (!m_worldRenderer || !m_scene)
		return;

	// Grab interpolated camera transform.
	if (m_cameraEntity)
	{
		const float cameraInterval = info.getInterval();
		m_worldRenderView.setView(
			m_worldRenderView.getView(),
			(m_cameraTransform.get(cameraInterval) * m_cameraOffset).inverse().toMatrix44()
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

	// Add render passes with world renderer.
	m_worldRenderer->setup(
		m_scene->getWorld(),
		m_worldRenderView,
		renderGraph,
		0,
		nullptr
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

	// Restore entity transforms captured before reconfiguration.
	if (m_scene)
	{
		for (auto entity : m_scene->getWorld()->getEntities())
		{
			if (auto persistentIdComponent = entity->getComponent< world::PersistentIdComponent >())
			{
				auto it = m_entityTransforms.find(persistentIdComponent->getId());
				if (it != m_entityTransforms.end())
					entity->setTransform(it->second);
			}
		}
	}

	m_entityTransforms.clear();
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

void WorldLayer::hotReload()
{
	// Capture transforms of all entities in the scene, so we
	// can restore then after reload.
	if (m_scene)
	{
		for (auto entity : m_scene->getWorld()->getEntities())
		{
			if (auto persistentIdComponent = entity->getComponent< world::PersistentIdComponent >())
				m_entityTransforms[persistentIdComponent->getId()] = entity->getTransform();
		}
	}
}

scene::Scene* WorldLayer::getScene() const
{
	return m_scene;
}

world::World* WorldLayer::getWorld() const
{
	return m_scene->getWorld();
}

world::IWorldRenderer* WorldLayer::getWorldRenderer() const
{
	return m_worldRenderer;
}

Ref< world::Entity > WorldLayer::createEntity(const Guid& entityDataId) const
{
	resource::Proxy< world::EntityData > entityData;
	if (!m_environment->getResource()->getResourceManager()->bind(resource::Id< world::EntityData >(entityDataId), entityData))
		return nullptr;

	const world::EntityBuilder entityBuilder(
		m_environment->getWorld()->getEntityFactory(),
		m_scene->getWorld()
	);
	return entityBuilder.create(entityData);
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
	const Vector4 viewPosition = m_worldRenderView.getView() * worldPosition;
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

void WorldLayer::setAlternateTime(double alternateTime)
{
	m_alternateTime = alternateTime;
}

double WorldLayer::getAlternateTime() const
{
	return m_alternateTime;
}

void WorldLayer::setCamera(const world::Entity* cameraEntity)
{
	m_cameraEntity = cameraEntity;
}

const world::Entity* WorldLayer::getCamera() const
{
	return m_cameraEntity;
}

}
