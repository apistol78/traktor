/*
 * TRAKTOR
 * Copyright (c) 2023-2024 Anders Pistol.
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
#include "Render/ScreenRenderer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Resource/IResourceManager.h"
#include "Runtime/IAudioServer.h"
#include "Runtime/IEnvironment.h"
#include "Runtime/UpdateInfo.h"
#include "Runtime/Engine/Stage.h"
#include "Runtime/Engine/SplitWorldLayer.h"
#include "Scene/Scene.h"
#include "Sound/Filters/SurroundEnvironment.h"
#include "World/IWorldRenderer.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity.h"
#include "World/EntityBuilder.h"
#include "World/EntityData.h"
#include "World/World.h"
#include "World/Entity/CameraComponent.h"
#include "World/Entity/GroupComponent.h"
#include "World/Entity/PersistentIdComponent.h"

namespace traktor::runtime
{
	namespace
	{

const render::Handle s_handleFeedback(L"Feedback");
const render::Handle s_handleSplitLeft(L"SplitLeft");
const render::Handle s_handleSplitRight(L"SplitRight");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.SplitWorldLayer", SplitWorldLayer, Layer)

SplitWorldLayer::SplitWorldLayer(
	Stage* stage,
	const std::wstring& name,
	bool permitTransition,
	IEnvironment* environment,
	const resource::Proxy< scene::Scene >& scene,
	const resource::Proxy< render::Shader >& shader
)
:	Layer(stage, name, permitTransition)
,	m_environment(environment)
,	m_scene(scene)
,	m_shader(shader)
{
	// Get initial field of view.
	m_fieldOfView = m_environment->getSettings()->getProperty< float >(L"World.FieldOfView", 70.0f);

	// Set index in each view.
	m_worldRenderViews[0].setIndex(0);
	m_worldRenderViews[1].setIndex(1);
}

void SplitWorldLayer::destroy()
{
	m_environment = nullptr;
	m_cameraEntities[0] = nullptr;
	m_cameraEntities[1] = nullptr;

	if (m_scene)
	{
		m_scene->destroy();
		m_scene.clear();
	}

	safeDestroy(m_worldRenderer);

	Layer::destroy();
}

void SplitWorldLayer::transition(Layer* fromLayer)
{
	const bool permit = fromLayer->isTransitionPermitted() && isTransitionPermitted();
	if (!permit)
		return;

	SplitWorldLayer* fromSplitWorldLayer = mandatory_non_null_type_cast< SplitWorldLayer* >(fromLayer);
	if (DeepHash(m_scene->getWorldRenderSettings()) == DeepHash(fromSplitWorldLayer->m_scene->getWorldRenderSettings()))
	{
		m_worldRenderer = fromSplitWorldLayer->m_worldRenderer;
		m_worldRenderViews[0] = fromSplitWorldLayer->m_worldRenderViews[0];
		m_worldRenderViews[1] = fromSplitWorldLayer->m_worldRenderViews[1];

		fromSplitWorldLayer->m_worldRenderer = nullptr;

		// Also need to ensure scene change doesn't reset world renderer.
		m_scene.consume();
	}
}

void SplitWorldLayer::preUpdate(const UpdateInfo& info)
{
	T_PROFILER_SCOPE(L"SplitWorldLayer pre-update");
	if (m_scene.changed())
	{
		// Scene has been successfully validated; drop existing world renderer if we've been flushed.
		m_worldRenderer = nullptr;
		m_scene.consume();

		// Get initial cameras.
		m_cameraEntities[0] = m_scene->getWorld()->getEntity(L"Camera0");
		m_cameraEntities[1] = m_scene->getWorld()->getEntity(L"Camera1");
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
	for (int32_t i = 0; i < 2; ++i)
	{
		m_worldRenderViews[i].setPerspective(
			float(width) / 2.0f,
			float(height),
			m_environment->getRender()->getAspectRatio() / 2.0f,
			deg2rad(m_fieldOfView),
			m_scene->getWorldRenderSettings()->viewNearZ,
			m_scene->getWorldRenderSettings()->viewFarZ
		);
	}

	// Set projection from camera component.
	for (int32_t i = 0; i < 2; ++i)
	{
		if (m_cameraEntities[i])
		{
			const world::CameraComponent* camera = m_cameraEntities[i]->getComponent< world::CameraComponent >();
			if (camera)
			{
				if (camera->getProjection() == world::Projection::Orthographic)
				{
					m_worldRenderViews[i].setOrthogonal(
						camera->getWidth(),
						camera->getHeight(),
						m_scene->getWorldRenderSettings()->viewNearZ,
						m_scene->getWorldRenderSettings()->viewFarZ
					);
				}
				else // Projection::Perspective
				{
					m_worldRenderViews[i].setPerspective(
						float(width) / 2.0f,
						float(height),
						m_environment->getRender()->getAspectRatio() / 2.0f,
						camera->getFieldOfView(),
						m_scene->getWorldRenderSettings()->viewNearZ,
						m_scene->getWorldRenderSettings()->viewFarZ
					);
				}
			}
		}
	}
}

void SplitWorldLayer::update(const UpdateInfo& info)
{
	T_PROFILER_SCOPE(L"SplitWorldLayer update");
	if (!m_worldRenderer)
		return;

	const double timeScale = m_environment->getControl()->getPause() ? 0.0 : 1.0;
	const double deltaTime = info.getSimulationDeltaTime() * timeScale;

	world::UpdateParams up;
	up.contextObject = getStage();
	up.totalTime = info.getSimulationTime();
	up.alternateTime = m_alternateTime;
	up.deltaTime = deltaTime;
	m_scene->update(up);

	// In case not explicitly set we update the alternative time also.
	m_alternateTime += deltaTime;
}

void SplitWorldLayer::postUpdate(const UpdateInfo& info)
{
	T_PROFILER_SCOPE(L"SplitWorldLayer post-update");
	if (!m_worldRenderer || !m_scene)
		return;

	// Update camera transform from entity.
	for (int32_t i = 0; i < 2; ++i)
	{
		if (m_cameraEntities[i])
		{
			const Transform cameraTransform = m_cameraEntities[i]->getTransform();
			m_cameraTransforms[i].step();
			m_cameraTransforms[i].set(cameraTransform);
		}
	}
}

void SplitWorldLayer::preSetup(const UpdateInfo& info)
{
	T_PROFILER_SCOPE(L"SplitWorldLayer pre-setup");
	if (!m_worldRenderer || !m_scene)
		return;

	// Grab interpolated camera transform.
	const float cameraInterval = info.getInterval();
	for (int32_t i = 0; i < 2; ++i)
	{
		if (m_cameraEntities[i])
		{
			m_worldRenderViews[i].setView(
				m_worldRenderViews[i].getView(),
				(m_cameraTransforms[i].get(cameraInterval) * m_cameraOffsets[i]).inverse().toMatrix44()
			);
		}
	}

	for (int32_t i = 0; i < 2; ++i)
	{
		m_worldRenderViews[i].setTimes(
			info.getStateTime(),
			info.getFrameDeltaTime(),
			info.getInterval()
		);
	}
}

void SplitWorldLayer::setup(const UpdateInfo& info, render::RenderGraph& renderGraph)
{
	T_PROFILER_SCOPE(L"SplitWorldLayer setup");
	if (!m_worldRenderer || !m_scene || !m_shader)
		return;

	if (!m_screenRenderer)
	{
		m_screenRenderer = new render::ScreenRenderer();
		m_screenRenderer->create(m_environment->getRender()->getRenderSystem());
	}

	// Build a root entity by gathering entities from containers.
	//auto group = m_rootGroup->getComponent< world::GroupComponent >();
	//group->removeAllEntities();

	//world::EventManagerComponent* eventManager = m_environment->getWorld()->getEntityEventManager();
	//if (eventManager)
	//	eventManager->gather([&](world::Entity* entity) { group->addEntity(entity); });

	//group->addEntity(m_scene->getRootEntity());
	//group->addEntity(m_dynamicEntities);

	// Get render view dimensions.
	const int32_t width = m_environment->getRender()->getWidth();
	const int32_t height = m_environment->getRender()->getHeight();

#if defined(__LINUX__)
	const int32_t denom = 1;
#else
	const int32_t denom = 1;
#endif
	render::RenderGraphTargetSetDesc rgtsd;
	rgtsd.count = 1;
	rgtsd.width = width / (2 * denom);
	rgtsd.height = height / denom;
	rgtsd.createDepthStencil = true;
	rgtsd.ignoreStencil = true;
	rgtsd.targets[0].colorFormat = render::TfR11G11B10F;

	auto leftTargetSetId = renderGraph.addTransientTargetSet(
		L"Split Left",
		rgtsd
	);
	auto rightTargetSetId = renderGraph.addTransientTargetSet(
		L"Split Right",
		rgtsd
	);

	m_worldRenderer->setup(
		m_scene->getWorld(),
		m_worldRenderViews[0],
		renderGraph,
		leftTargetSetId,
		nullptr
	);
	m_worldRenderer->setup(
		m_scene->getWorld(),
		m_worldRenderViews[1],
		renderGraph,
		rightTargetSetId,
		nullptr
	);

	Ref< render::RenderPass > rp = new render::RenderPass(L"Split");

	render::Clear cl;
	rp->setOutput(0, cl, render::TfNone, render::TfColor);

	rp->addInput(leftTargetSetId);
	rp->addInput(rightTargetSetId);

	rp->addBuild([=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {

		auto leftTargetSet = renderGraph.getTargetSet(leftTargetSetId);
		auto rightTargetSet = renderGraph.getTargetSet(rightTargetSetId);

		auto programParams = renderContext->alloc< render::ProgramParameters >();
		programParams->beginParameters(renderContext);
		programParams->setTextureParameter(s_handleSplitLeft, leftTargetSet->getColorTexture(0));
		programParams->setTextureParameter(s_handleSplitRight, rightTargetSet->getColorTexture(0));
		programParams->endParameters(renderContext);

		m_screenRenderer->draw(renderContext, m_shader, programParams);

	});
	renderGraph.addPass(rp);
}

void SplitWorldLayer::preReconfigured()
{
	// Destroy previous world renderer; do this
	// before re-configuration of servers as world
	// renderer might have heavy resources already created
	// such as render targets and textures.
	safeDestroy(m_worldRenderer);
}

void SplitWorldLayer::postReconfigured()
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

void SplitWorldLayer::suspend()
{
#if defined(__ANDROID__) || defined(__IOS__)
	// Destroy previous world renderer; do this
	// to save memory which is easily re-constructed at resume.
	safeDestroy(m_worldRenderer);
#endif
}

void SplitWorldLayer::resume()
{
}

void SplitWorldLayer::hotReload()
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

scene::Scene* SplitWorldLayer::getScene() const
{
	return m_scene;
}

world::World* SplitWorldLayer::getWorld() const
{
	return m_scene->getWorld();
}

world::IWorldRenderer* SplitWorldLayer::getWorldRenderer() const
{
	return m_worldRenderer;
}

Ref< world::Entity > SplitWorldLayer::createEntity(const Guid& entityDataId) const
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

void SplitWorldLayer::setControllerEnable(bool controllerEnable)
{
	m_controllerEnable = controllerEnable;
}

void SplitWorldLayer::resetController()
{
}

const Frustum& SplitWorldLayer::getViewFrustum(int32_t split) const
{
	return m_worldRenderViews[split].getViewFrustum();
}

bool SplitWorldLayer::worldToView(int32_t split, const Vector4& worldPosition, Vector4& outViewPosition) const
{
	outViewPosition = m_worldRenderViews[split].getView() * worldPosition;
	return true;
}

bool SplitWorldLayer::viewToWorld(int32_t split, const Vector4& viewPosition, Vector4& outWorldPosition) const
{
	outWorldPosition = m_worldRenderViews[split].getView().inverse() * viewPosition;
	return true;
}

bool SplitWorldLayer::worldToScreen(int32_t split, const Vector4& worldPosition, Vector2& outScreenPosition) const
{
	const Vector4 viewPosition = m_worldRenderViews[split].getView() * worldPosition;
	return viewToScreen(split, viewPosition, outScreenPosition);
}

bool SplitWorldLayer::viewToScreen(int32_t split, const Vector4& viewPosition, Vector2& outScreenPosition) const
{
	Vector4 clipPosition = m_worldRenderViews[split].getProjection() * viewPosition;
	if (clipPosition.w() <= 0.0f)
		return false;
	clipPosition /= clipPosition.w();
	outScreenPosition = Vector2(clipPosition.x(), clipPosition.y());
	return true;
}

bool SplitWorldLayer::screenToView(int32_t split, const Vector2& screenPosition, Ray3& outViewRay) const
{
	const Frustum& viewFrustum = m_worldRenderViews[split].getViewFrustum();

	const Vector4 t = lerp(viewFrustum.corners[0], viewFrustum.corners[1], Scalar(screenPosition.x * 0.5f + 0.5f));
	const Vector4 b = lerp(viewFrustum.corners[3], viewFrustum.corners[2], Scalar(screenPosition.x * 0.5f + 0.5f));
	const Vector4 p = lerp(b, t, Scalar(screenPosition.y * 0.5f + 0.5f));

	outViewRay.origin = Vector4::origo();
	outViewRay.direction = p.xyz0().normalized();
	return true;
}

bool SplitWorldLayer::screenToWorld(int32_t split, const Vector2& screenPosition, Ray3& outWorldRay) const
{
	Ray3 viewRay;
	if (!screenToView(split, screenPosition, viewRay))
		return false;

	const Matrix44 viewInverse = m_worldRenderViews[split].getView().inverse();
	outWorldRay.origin = viewInverse * viewRay.origin;
	outWorldRay.direction = viewInverse * viewRay.direction;
	return true;
}

void SplitWorldLayer::setFieldOfView(float fieldOfView)
{
	m_fieldOfView = fieldOfView;
}

float SplitWorldLayer::getFieldOfView() const
{
	return m_fieldOfView;
}

void SplitWorldLayer::setAlternateTime(double alternateTime)
{
	m_alternateTime = alternateTime;
}

double SplitWorldLayer::getAlternateTime() const
{
	return m_alternateTime;
}

void SplitWorldLayer::setCamera(int32_t split, const world::Entity* cameraEntity)
{
	m_cameraEntities[split] = cameraEntity;
}

const world::Entity* SplitWorldLayer::getCamera(int32_t split) const
{
	return m_cameraEntities[split];
}

}
