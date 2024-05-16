/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/AnimatedMeshComponent.h"
#include "Animation/AnimationResourceFactory.h"
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonComponent.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/Editor/AnimationPreviewControl.h"
#include "Core/Math/Plane.h"
#include "Core/Misc/ObjectStore.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Editor/IEditor.h"
#include "Mesh/MeshComponentRenderer.h"
#include "Mesh/MeshEntityFactory.h"
#include "Mesh/MeshResourceFactory.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraphFactory.h"
#include "Render/Resource/AliasTextureFactory.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/TextureFactory.h"
#include "Resource/ResourceManager.h"
#include "Scene/Scene.h"
#include "Scene/SceneFactory.h"
#include "Ui/Application.h"
#include "Ui/Itf/IWidget.h"
#include "Weather/WeatherFactory.h"
#include "Weather/Sky/SkyRenderer.h"
#include "World/Entity.h"
#include "World/EntityFactory.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldResourceFactory.h"
#include "World/Entity/CullingRenderer.h"
#include "World/Entity/GroupComponent.h"
#include "World/Entity/LightComponent.h"
#include "World/Entity/ProbeRenderer.h"
#include "World/Entity/VolumetricFogRenderer.h"
#include "World/Entity/WorldEntityFactory.h"
#include "World/Forward/WorldRendererForward.h"

namespace traktor::animation
{
	namespace
	{

const resource::Id< scene::Scene > c_previewScene(L"{84ADD065-E963-9D4D-A28D-FF44BD616B0F}");

const float c_deltaMoveScale = 0.025f;
const float c_deltaScaleHead = 0.015f;
const float c_deltaScalePitch = 0.005f;

world::IEntityFactory* initializeFactory(world::IEntityFactory* entityFactory, const ObjectStore& objectStore)
{
	return entityFactory->initialize(objectStore) ? entityFactory : nullptr;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimationPreviewControl", AnimationPreviewControl, ui::Widget)

AnimationPreviewControl::AnimationPreviewControl(editor::IEditor* editor)
:	m_editor(editor)
,	m_position(0.0f, -2.0f, 7.0f, 1.0f)
,	m_angleHead(0.0f)
,	m_anglePitch(0.0f)
{
}

bool AnimationPreviewControl::create(ui::Widget* parent)
{
	if (!Widget::create(parent, ui::WsNoCanvas))
		return false;

	m_renderSystem = m_editor->getObjectStore()->get< render::IRenderSystem >();
	if (!m_renderSystem)
		return false;

	Ref< db::Database > resourceDatabase = m_editor->getOutputDatabase();
	if (!resourceDatabase)
		return false;

	m_resourceManager = new resource::ResourceManager(resourceDatabase, m_editor->getSettings()->getProperty< bool >(L"Resource.Verbose", false));

	// Setup object store with relevant systems.
	ObjectStore objectStore;
	objectStore.set(m_resourceManager);
	objectStore.set(m_renderSystem);

	Ref< world::EntityFactory > entityFactory = new world::EntityFactory();
	entityFactory->addFactory(initializeFactory(new world::WorldEntityFactory(true), objectStore));
	entityFactory->addFactory(initializeFactory(new weather::WeatherFactory(), objectStore));
	entityFactory->addFactory(initializeFactory(new mesh::MeshEntityFactory(), objectStore));

	m_resourceManager->addFactory(new AnimationResourceFactory());
	m_resourceManager->addFactory(new mesh::MeshResourceFactory(m_renderSystem));
	m_resourceManager->addFactory(new render::AliasTextureFactory());
	m_resourceManager->addFactory(new render::ShaderFactory(m_renderSystem));
	m_resourceManager->addFactory(new render::TextureFactory(m_renderSystem, 0));
	m_resourceManager->addFactory(new render::ImageGraphFactory(m_renderSystem));
	m_resourceManager->addFactory(new scene::SceneFactory(m_renderSystem, entityFactory));
	m_resourceManager->addFactory(new world::WorldResourceFactory(m_renderSystem, nullptr));
	
	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 0;
	desc.multiSample = m_editor->getSettings()->getProperty< int32_t >(L"Editor.MultiSample", 4);
	desc.waitVBlanks = 1;
	desc.syswin = getIWidget()->getSystemWindow();

	m_renderView = m_renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_renderContext = new render::RenderContext(4 * 1024 * 1024);
	m_renderGraph = new render::RenderGraph(m_renderSystem, desc.multiSample);

	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(m_resourceManager, m_renderSystem, 1))
		return false;

	if (!m_resourceManager->bind(c_previewScene, m_sceneInstance))
		return false;

	m_sceneInstance.consume();

	addEventHandler< ui::MouseButtonDownEvent >(this, &AnimationPreviewControl::eventButtonDown);
	addEventHandler< ui::MouseButtonUpEvent >(this, &AnimationPreviewControl::eventButtonUp);
	addEventHandler< ui::MouseMoveEvent >(this, &AnimationPreviewControl::eventMouseMove);
	addEventHandler< ui::SizeEvent >(this, &AnimationPreviewControl::eventSize);
	addEventHandler< ui::PaintEvent >(this, &AnimationPreviewControl::eventPaint);

	updateSettings();
	updateWorldRenderer();

	m_idleEventHandler = ui::Application::getInstance()->addEventHandler< ui::IdleEvent >(this, &AnimationPreviewControl::eventIdle);

	m_timer.reset();
	return true;
}

void AnimationPreviewControl::destroy()
{
	ui::Application::getInstance()->removeEventHandler< ui::IdleEvent >(m_idleEventHandler);

	m_sceneInstance.clear();
	m_mesh.clear();
	m_skeleton.clear();

	m_poseController = nullptr;
	m_entity = nullptr;

	safeDestroy(m_primitiveRenderer);
	safeDestroy(m_resourceManager);
	safeDestroy(m_renderGraph);
	safeDestroy(m_worldRenderer);
	safeClose(m_renderView);

	Widget::destroy();
}

void AnimationPreviewControl::setMesh(const resource::Id< mesh::SkinnedMesh >& mesh)
{
	m_resourceManager->bind(mesh, m_mesh);
	updatePreview();
}

void AnimationPreviewControl::setSkeleton(const resource::Id< Skeleton >& skeleton)
{
	m_resourceManager->bind(skeleton, m_skeleton);
	updatePreview();
}

void AnimationPreviewControl::setPoseController(IPoseController* poseController)
{
	m_poseController = poseController;
	updatePreview();
}

void AnimationPreviewControl::updateSettings()
{
	Ref< PropertyGroup > colors = m_editor->getSettings()->getProperty< PropertyGroup >(L"Editor.Colors");
	m_colorClear = colors->getProperty< Color4ub >(L"Background");
	m_colorGrid = colors->getProperty< Color4ub >(L"Grid");
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
		m_poseController
	);

	Ref< AnimatedMeshComponent > meshComponent = new AnimatedMeshComponent(
		Transform::identity(),
		m_mesh,
		m_renderSystem
	);

	m_entity = new world::Entity();
	m_entity->setComponent(skeletonComponent);
	m_entity->setComponent(meshComponent);
}

void AnimationPreviewControl::updateWorldRenderer()
{
	safeDestroy(m_worldRenderer);

	Ref< world::WorldEntityRenderers > worldEntityRenderers = new world::WorldEntityRenderers();
	worldEntityRenderers->add(new mesh::MeshComponentRenderer());
	worldEntityRenderers->add(new weather::SkyRenderer());
	worldEntityRenderers->add(new world::CullingRenderer());
	worldEntityRenderers->add(new world::VolumetricFogRenderer());
	worldEntityRenderers->add(new world::ProbeRenderer(
		m_resourceManager,
		m_renderSystem,
		type_of< world::WorldRendererForward >()
	));

	world::WorldCreateDesc wcd;
	wcd.worldRenderSettings = m_sceneInstance->getWorldRenderSettings();
	wcd.entityRenderers = worldEntityRenderers;

	Ref< world::IWorldRenderer > worldRenderer = new world::WorldRendererForward();
	if (!worldRenderer->create(
		m_resourceManager,
		m_renderSystem,
		wcd
	))
		return;

	m_worldRenderer = worldRenderer;
}

void AnimationPreviewControl::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	m_lastMousePosition = event->getPosition();
	setCapture();
}

void AnimationPreviewControl::eventButtonUp(ui::MouseButtonUpEvent* event)
{
	releaseCapture();
}

void AnimationPreviewControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	if (!hasCapture())
		return;

	if (event->getButton() == ui::MbtLeft)
	{
		if ((event->getKeyState() & ui::KsControl) == 0)
		{
			// Move X/Z direction.
			const float dx = -float(m_lastMousePosition.x - event->getPosition().x) * c_deltaMoveScale;
			const float dz = -float(m_lastMousePosition.y - event->getPosition().y) * c_deltaMoveScale;
			m_position += Vector4(dx, 0.0f, dz, 0.0f);
		}
		else
		{
			// Move X/Y direction.
			const float dx = -float(m_lastMousePosition.x - event->getPosition().x) * c_deltaMoveScale;
			const float dy =  float(m_lastMousePosition.y - event->getPosition().y) * c_deltaMoveScale;
			m_position += Vector4(dx, dy, 0.0f, 0.0f);
		}
	}
	else if (event->getButton() == ui::MbtRight)
	{
		m_angleHead += float(m_lastMousePosition.x - event->getPosition().x) * c_deltaScaleHead;
		m_anglePitch += float(m_lastMousePosition.y - event->getPosition().y) * c_deltaScalePitch;
	}

	m_lastMousePosition = event->getPosition();

	update();
}

void AnimationPreviewControl::eventSize(ui::SizeEvent* event)
{
	if (!m_renderView)
		return;

	const ui::Size sz = event->getSize();
	m_renderView->reset(sz.cx, sz.cy);
}

void AnimationPreviewControl::eventPaint(ui::PaintEvent* event)
{
	// Reload scene if changed.
	if (m_sceneInstance.changed())
	{
		safeDestroy(m_worldRenderer);
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

	// Render view events; reset view if it has become lost.
	bool lost = false;
	for (render::RenderEvent re = {}; m_renderView->nextEvent(re); )
	{
		if (re.type == render::RenderEventType::Lost)
			lost = true;
	}

	const ui::Size sz = getInnerRect().getSize();
	if (lost || sz.cx != m_dirtySize.cx || sz.cy != m_dirtySize.cy)
	{
		if (!m_renderView->reset(sz.cx, sz.cy))
			return;
		m_dirtySize = sz;
	}

	const double time = m_timer.getElapsedTime();
	const double scaledTime = m_timer.getElapsedTime();
	const double deltaTime = m_timer.getDeltaTime();

	float tmp[4];
	m_colorClear.getRGBA32F(tmp);
	const Color4f clearColor(tmp[0], tmp[1], tmp[2], tmp[3]);

	const float aspect = float(sz.cx) / sz.cy;

	const Matrix44 viewTransform = translate(m_position) * rotateX(m_anglePitch) * rotateY(m_angleHead);
	const Matrix44 projectionTransform = perspectiveLh(
		65.0f * PI / 180.0f,
		aspect,
		0.1f,
		1000.0f
	);

	const Matrix44 viewInverse = viewTransform.inverse();
	const Plane cameraPlane(
		viewInverse.axisZ(),
		viewInverse.translation()
	);

	// Update scene entities.
	world::UpdateParams update;
	update.totalTime = time;
	update.alternateTime = time;
	update.deltaTime = deltaTime;
	m_sceneInstance->update(update);

	// Build a root entity by gathering entities from containers.
	//Ref< world::GroupComponent > rootGroup = new world::GroupComponent();
	//Ref< world::Entity > rootEntity = new world::Entity();
	//rootEntity->setComponent(rootGroup);

	//rootGroup->addEntity(m_sceneInstance->getRootEntity());

	//// Update and add animated mesh entity.
	//if (m_entity)
	//{
	//	m_entity->update(update);
	//	rootGroup->addEntity(m_entity);
	//}

	// Setup world render passes.
	const world::WorldRenderSettings* worldRenderSettings = m_sceneInstance->getWorldRenderSettings();
	m_worldRenderView.setPerspective(
		float(sz.cx),
		float(sz.cy),
		float(sz.cx) / sz.cy,
		deg2rad(70.0f),
		worldRenderSettings->viewNearZ,
		worldRenderSettings->viewFarZ
	);
	m_worldRenderView.setTimes(time, deltaTime, 1.0f);
	m_worldRenderView.setView(m_worldRenderView.getView(), viewTransform);
	m_worldRenderer->setup(m_sceneInstance->getWorld(), m_worldRenderView, *m_renderGraph, 0, nullptr);

	// Draw debug wires.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Debug wire");
	rp->setOutput(0, render::TfAll, render::TfAll);
	rp->addBuild([&](const render::RenderGraph&, render::RenderContext* renderContext) {
		m_primitiveRenderer->begin(0, projectionTransform);
		m_primitiveRenderer->pushView(viewTransform);

		for (int x = -10; x <= 10; ++x)
		{
			m_primitiveRenderer->drawLine(
				Vector4(float(x), 0.0f, -10.0f, 1.0f),
				Vector4(float(x), 0.0f, 10.0f, 1.0f),
				(x == 0) ? 1.0f : 0.0f,
				m_colorGrid
			);
			m_primitiveRenderer->drawLine(
				Vector4(-10.0f, 0.0f, float(x), 1.0f),
				Vector4(10.0f, 0.0f, float(x), 1.0f),
				(x == 0) ? 1.0f : 0.0f,
				m_colorGrid
			);
		}

		m_primitiveRenderer->end(0);

		auto rb = renderContext->allocNamed< render::LambdaRenderBlock >(L"Debug wire");
		rb->lambda = [&](render::IRenderView* renderView) {
			m_primitiveRenderer->render(m_renderView, 0);
		};
		renderContext->draw(rb);
	});
	m_renderGraph->addPass(rp);

	// Validate render graph.
	if (!m_renderGraph->validate())
		return;

	// Build render context.
	m_renderContext->flush();
	m_renderGraph->build(m_renderContext, m_dirtySize.cx, m_dirtySize.cy);

	// Render frame.
	if (m_renderView->beginFrame())
	{
		m_renderContext->render(m_renderView);
		m_renderView->endFrame();
		m_renderView->present();
	}

	// Need to clear all entities from our root group since when our root entity
	// goes out of scope it's automatically destroyed.
	//rootGroup->removeAllEntities();

	event->consume();
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
