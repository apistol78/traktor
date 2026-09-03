/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/AnimationPreviewControl.h"

#include "Animation/AnimatedMeshComponent.h"
#include "Animation/Animation/AnimationGraphPoseController.h"
#include "Animation/Animation/RtStateGraphResourceFactory.h"
#include "Animation/AnimationResourceFactory.h"
#include "Animation/Editor/SkeletonRenderer.h"
#include "Animation/Joint.h"
#include "Animation/RagDoll/RagDollResourceFactory.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonComponent.h"
#include "Animation/SkeletonUtils.h"
#include "Core/Containers/SmallSet.h"
#include "Core/Guid.h"
#include "Core/Math/Plane.h"
#include "Core/Math/Transform.h"
#include "Core/Misc/ObjectStore.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IEditor.h"
#include "Mesh/MeshEntityFactory.h"
#include "Mesh/MeshResourceFactory.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Skinned/SkinnedMeshComponentRenderer.h"
#include "Mesh/Static/StaticMeshComponentRenderer.h"
#include "Physics/Body.h"
#include "Physics/BoxShapeDesc.h"
#include "Physics/CollisionSpecification.h"
#include "Physics/PhysicsFactory.h"
#include "Physics/PhysicsManager.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/World/EntityFactory.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraphFactory.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Render/Resource/AliasTextureFactory.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/TextureFactory.h"
#include "Resource/ResourceManager.h"
#include "Scene/Scene.h"
#include "Scene/SceneFactory.h"
#include "Ui/Application.h"
#include "Ui/Itf/IWidget.h"
#include "Weather/Sky/SkyRenderer.h"
#include "Weather/WeatherFactory.h"
#include "World/Deferred/WorldRendererDeferred.h"
#include "World/Entity.h"
#include "World/Entity/CullingRenderer.h"
#include "World/Entity/GroupComponent.h"
#include "World/Entity/LightComponent.h"
#include "World/Entity/ProbeRenderer.h"
#include "World/Entity/RTWorldRenderer.h"
#include "World/Entity/WorldEntityFactory.h"
#include "World/EntityFactory.h"
#include "World/World.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldResourceFactory.h"

namespace traktor::animation
{
namespace
{

const resource::Id< scene::Scene > c_previewScene(L"{02ABDE03-E9F2-45A8-8FAA-3772EC7568ED}");


world::IEntityFactory* initializeFactory(world::IEntityFactory* entityFactory, const ObjectStore& objectStore)
{
	return entityFactory->initialize(objectStore) ? entityFactory : nullptr;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimationPreviewControl", AnimationPreviewControl, render::RenderControl)

AnimationPreviewControl::AnimationPreviewControl(editor::IEditor* editor)
	: m_editor(editor)
{
}

bool AnimationPreviewControl::create(ui::Widget* parent)
{
	if (!render::RenderControl::create(parent, m_editor))
		return false;

	render::IRenderSystem* renderSystem = getRenderSystem();

	const std::wstring physicsManagerTypeName = m_editor->getSettings()->getProperty< std::wstring >(L"SceneEditor.PhysicsManager");
	const TypeInfo* physicsManagerType = TypeInfo::find(physicsManagerTypeName.c_str());
	if (!physicsManagerType)
		return false;

	physics::PhysicsCreateDesc pcd;
	pcd.timeScale = 1.0f;
	pcd.solverIterations = 10;

	m_physicsManager = checked_type_cast< physics::PhysicsManager* >(physicsManagerType->createInstance());
	if (!m_physicsManager)
		return false;

	if (!m_physicsManager->create(pcd))
		return false;

	m_physicsManager->setGravity(Vector4(0.0f, -9.81f, 0.0f, 0.0f));

	// Setup object store with relevant systems.
	ObjectStore objectStore;
	objectStore.set(getResourceManager());
	objectStore.set(m_physicsManager);
	objectStore.set(renderSystem);

	Ref< world::EntityFactory > entityFactory = new world::EntityFactory();
	entityFactory->addFactory(initializeFactory(new world::WorldEntityFactory(true), objectStore));
	entityFactory->addFactory(initializeFactory(new weather::WeatherFactory(), objectStore));
	entityFactory->addFactory(initializeFactory(new mesh::MeshEntityFactory(), objectStore));
	entityFactory->addFactory(initializeFactory(new physics::EntityFactory(), objectStore));

	// Texture and shader factories are already added by the base control.
	getResourceManager()->addFactory(new AnimationResourceFactory());
	getResourceManager()->addFactory(new RagDollResourceFactory());
	getResourceManager()->addFactory(new RtStateGraphResourceFactory());
	getResourceManager()->addFactory(new mesh::MeshResourceFactory(renderSystem));
	getResourceManager()->addFactory(new physics::PhysicsFactory());
	getResourceManager()->addFactory(new render::AliasTextureFactory());
	getResourceManager()->addFactory(new render::ImageGraphFactory(renderSystem));
	getResourceManager()->addFactory(new scene::SceneFactory(renderSystem, entityFactory));
	getResourceManager()->addFactory(new world::WorldResourceFactory(renderSystem, nullptr));

	m_renderContext = new render::RenderContext(4 * 1024 * 1024);
	m_renderGraph = new render::RenderGraph(renderSystem, getMultiSample());

	if (!getResourceManager()->bind(c_previewScene, m_sceneInstance))
		return false;

	m_sceneInstance.consume();

	updateSettings();
	updateWorldRenderer();

	m_idleEventHandler = ui::Application::getInstance()->addEventHandler< ui::IdleEvent >(this, &AnimationPreviewControl::eventIdle);

	m_timer.reset();
	return true;
}

void AnimationPreviewControl::destroy()
{
	ui::Application::getInstance()->removeEventHandler(m_idleEventHandler);

	m_sceneInstance.clear();
	m_mesh.clear();
	m_skeleton.clear();

	m_poseController = nullptr;
	m_entity = nullptr;

	safeDestroy(m_physicsManager);
	safeDestroy(m_renderGraph);
	safeDestroy(m_worldRenderer);

	render::RenderControl::destroy();
}

void AnimationPreviewControl::setMesh(const resource::Id< mesh::SkinnedMesh >& mesh)
{
	getResourceManager()->bind(mesh, m_mesh);
	updatePreview();
}

void AnimationPreviewControl::setSkeleton(const resource::Id< Skeleton >& skeleton)
{
	getResourceManager()->bind(skeleton, m_skeleton);
	updatePreview();
}

void AnimationPreviewControl::setPoseController(IPoseController* poseController)
{
	m_poseController = poseController;
	updatePreview();
}

const Skeleton* AnimationPreviewControl::getSkeleton() const
{
	return m_skeleton;
}

void AnimationPreviewControl::setParameterValue(const std::wstring& parameterName, bool value)
{
	AnimationGraphPoseController* poseController = dynamic_type_cast< AnimationGraphPoseController* >(m_poseController);
	if (poseController)
		poseController->setParameterValue(render::Handle(parameterName.c_str()), value);
}

void AnimationPreviewControl::updateSettings()
{
	Ref< PropertyGroup > colors = m_editor->getSettings()->getProperty< PropertyGroup >(L"Editor.Colors");
	m_colorClear = colors->getProperty< Color4ub >(L"Background");
}

void AnimationPreviewControl::updatePreview()
{
	m_entity = nullptr;

	if (!m_mesh)
		return;

	AlignedVector< int32_t > jointRemap;
	if (m_skeleton)
	{
		jointRemap.resize(m_skeleton->getJointCount());

		const auto& jointMap = m_mesh->getJointMap();
		for (uint32_t i = 0; i < m_skeleton->getJointCount(); ++i)
		{
			const Joint* joint = m_skeleton->getJoint(i);
			T_ASSERT(joint);

			auto it = jointMap.find(joint->getName());
			if (it == jointMap.end())
			{
				jointRemap[i] = -1;
				continue;
			}

			jointRemap[i] = it->second;
		}
	}

	Ref< SkeletonComponent > skeletonComponent = new SkeletonComponent(
		Transform::identity(),
		m_skeleton,
		m_poseController);

	Ref< AnimatedMeshComponent > meshComponent = new AnimatedMeshComponent(
		Transform::identity(),
		m_mesh,
		getRenderSystem());

	m_entity = new world::Entity();
	m_entity->setComponent(skeletonComponent);
	m_entity->setComponent(meshComponent);
	m_entity->setState(world::EntityState::Dynamic, world::EntityState::Dynamic, true);
}

void AnimationPreviewControl::updateWorldRenderer()
{
	safeDestroy(m_worldRenderer);

	Ref< world::WorldEntityRenderers > worldEntityRenderers = new world::WorldEntityRenderers();
	worldEntityRenderers->add(new mesh::StaticMeshComponentRenderer());
	worldEntityRenderers->add(new mesh::SkinnedMeshComponentRenderer());
	worldEntityRenderers->add(new weather::SkyRenderer());
	worldEntityRenderers->add(new world::CullingRenderer());
	worldEntityRenderers->add(new world::ProbeRenderer(
		getResourceManager(),
		getRenderSystem(),
		type_of< world::WorldRendererDeferred >()));
	worldEntityRenderers->add(new world::RTWorldRenderer());

	world::WorldCreateDesc wcd;
	wcd.worldRenderSettings = m_sceneInstance->getWorldRenderSettings();
	wcd.entityRenderers = worldEntityRenderers;

	// Use same quality settings as scene editor.
	// #fixme Quality settings should probably be a general editor configuration.
	const PropertyGroup* settings = m_editor->getSettings();
	wcd.quality.imageProcess = (world::Quality)settings->getProperty< int32_t >(L"SceneEditor.PostProcessQuality", 4);
	wcd.quality.motionBlur = world::Quality::Disabled;
	wcd.quality.shadows = (world::Quality)settings->getProperty< int32_t >(L"SceneEditor.ShadowQuality", 4);
	wcd.quality.reflections = (world::Quality)settings->getProperty< int32_t >(L"SceneEditor.ReflectionsQuality", 4);
	wcd.quality.ambientOcclusion = (world::Quality)settings->getProperty< int32_t >(L"SceneEditor.AmbientOcclusionQuality", 4);
	wcd.quality.antiAlias = (world::Quality)settings->getProperty< int32_t >(L"SceneEditor.AntiAliasQuality", 4);
	wcd.quality.irradiance = (world::Quality)settings->getProperty< int32_t >(L"SceneEditor.IrradianceQuality", 4);

	wcd.hdr = getRenderView()->isHDR();

	Ref< world::IWorldRenderer > worldRenderer = new world::WorldRendererDeferred();
	if (!worldRenderer->create(
			getResourceManager(),
			getRenderSystem(),
			wcd))
	{
		safeDestroy(worldRenderer);
		return;
	}

	m_worldRenderer = worldRenderer;
}

bool AnimationPreviewControl::renderFrame()
{
	// Reload scene if changed.
	if (m_sceneInstance.changed())
	{
		safeDestroy(m_worldRenderer);
		m_sceneInstance.consume();
	}

	if (!m_sceneInstance)
		return false;

	// Lazy create world renderer.
	if (!m_worldRenderer)
	{
		updateWorldRenderer();
		if (!m_worldRenderer)
			return false;
	}

	if (!validateRenderView())
		return false;

	const ui::Size sz = getRenderSize();

	const double time = m_timer.getElapsedTime();
	const double scaledTime = m_timer.getElapsedTime();
	const double deltaTime = m_timer.getDeltaTime();

	float tmp[4];
	m_colorClear.getRGBA32F(tmp);
	const Color4f clearColor(tmp[0], tmp[1], tmp[2], tmp[3]);

	const Matrix44 viewTransform = getViewTransform();

	const Matrix44 viewInverse = viewTransform.inverse();
	const Plane cameraPlane(
		viewInverse.axisZ(),
		viewInverse.translation());

	// Temporarily add mesh entity to world.
	if (m_entity)
	{
		m_entity->setTransform(Transform::identity());
		m_sceneInstance->getWorld()->addEntity(m_entity);
	}

	// Step the physics simulation.
	if (m_physicsManager)
	{
		double stepDeltaTime = deltaTime;
		if (stepDeltaTime > 1.0 / 30.0)
			stepDeltaTime = 1.0 / 30.0;
		m_physicsManager->update((float)stepDeltaTime, false);
	}

	// Update scene entities.
	world::UpdateParams update;
	update.totalTime = time;
	update.alternateTime = time;
	update.deltaTime = deltaTime;
	m_sceneInstance->update(update);

	// Setup world render passes.
	const world::WorldRenderSettings* worldRenderSettings = m_sceneInstance->getWorldRenderSettings();
	m_worldRenderView.setPerspective(
		float(sz.cx),
		float(sz.cy),
		float(sz.cx) / sz.cy,
		deg2rad(70.0f),
		worldRenderSettings->viewNearZ,
		worldRenderSettings->viewFarZ);
	m_worldRenderView.setTimes(time, deltaTime, 1.0f);
	m_worldRenderView.setView(m_worldRenderView.getView(), viewTransform);
	m_worldRenderer->setup(m_sceneInstance->getWorld(), m_worldRenderView, *m_renderGraph, render::RGTargetSet::Output, nullptr);

	// Remove mesh entity from world.
	if (m_entity)
		m_sceneInstance->getWorld()->removeEntity(m_entity);

	// Draw debug wires.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Debug wire");
	rp->setOutput(render::RGTargetSet::Output, render::TfAll, render::TfAll);
	rp->addBuild([&](const render::RenderGraph&, render::RenderContext* renderContext) {
		// Use the same projection and view as the world (mesh) render so the debug
		// skeleton aligns with the mesh. Previously the primitive renderer used a
		// different field-of-view (65 vs 70 deg) and near/far, offsetting the overlay.
		getPrimitiveRenderer()->begin(0, m_worldRenderView.getProjection());
		getPrimitiveRenderer()->pushView(m_worldRenderView.getView());

		drawGrid(getPrimitiveRenderer(), 10.0f, 1.0f);

		// Draw skeleton current pose.
		if (m_entity)
		{
			if (const SkeletonComponent* skeletonComponent = m_entity->getComponent< SkeletonComponent >())
				drawSkeleton(getPrimitiveRenderer(), skeletonComponent, Matrix44::identity(), false, true);
		}

		getPrimitiveRenderer()->end(0);

		auto rb = renderContext->allocNamed< render::LambdaRenderBlock >(L"Debug wire");
		rb->lambda = [&](render::IRenderView* renderView) {
			getPrimitiveRenderer()->render(getRenderView(), 0);
		};
		renderContext->draw(rb);
	});
	m_renderGraph->addPass(rp);

	// Validate render graph.
	if (!m_renderGraph->validate())
		return false;

	// Build render context.
	m_renderGraph->build(m_renderContext, sz.cx, sz.cy);

	// Render frame.
	render::IRenderView* renderView = getRenderView();
	if (!renderView->beginFrame())
		return false;

	m_renderContext->render(renderView);
	m_renderContext->flush();

	renderView->endFrame();
	renderView->present();

	return true;
}

void AnimationPreviewControl::eventIdle(ui::IdleEvent* event)
{
	if (isVisible(true))
	{
		update();
		event->requestMore();
	}
}

}
