#include "Animation/AnimatedMeshComponent.h"
#include "Animation/AnimationResourceFactory.h"
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/Editor/AnimationPreviewControl.h"
#include "Core/Math/Plane.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Editor/IEditor.h"
#include "Mesh/MeshComponentRenderer.h"
#include "Mesh/MeshFactory.h"
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
#include "Ui/Application.h"
#include "Ui/Itf/IWidget.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity.h"
#include "World/EntityRenderer.h"
#include "World/Entity/GroupRenderer.h"
#include "World/Entity/LightComponent.h"
#include "World/Entity/LightRenderer.h"
#include "World/Forward/WorldRendererForward.h"

namespace traktor
{
	namespace animation
	{
		namespace
		{

const float c_deltaMoveScale = 0.025f;
const float c_deltaScaleHead = 0.015f;
const float c_deltaScalePitch = 0.005f;

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

	m_renderSystem = m_editor->getStoreObject< render::IRenderSystem >(L"RenderSystem");
	if (!m_renderSystem)
		return false;

	Ref< db::Database > resourceDatabase = m_editor->getOutputDatabase();
	if (!resourceDatabase)
		return false;

	m_resourceManager = new resource::ResourceManager(resourceDatabase, m_editor->getSettings()->getProperty< bool >(L"Resource.Verbose", false));
	m_resourceManager->addFactory(new AnimationResourceFactory());
	m_resourceManager->addFactory(new mesh::MeshFactory(m_renderSystem));
	m_resourceManager->addFactory(new render::AliasTextureFactory());
	m_resourceManager->addFactory(new render::ShaderFactory(m_renderSystem));
	m_resourceManager->addFactory(new render::TextureFactory(m_renderSystem, 0));
	m_resourceManager->addFactory(new render::ImageGraphFactory(m_renderSystem));

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 24;
	desc.stencilBits = 0;
	desc.multiSample = m_editor->getSettings()->getProperty< int32_t >(L"Editor.MultiSample", 4);
	desc.waitVBlanks = 0;
	desc.syswin = getIWidget()->getSystemWindow();

	m_renderView = m_renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_renderContext = new render::RenderContext(4 * 1024 * 1024);
	m_renderGraph = new render::RenderGraph(m_renderSystem, desc.multiSample);

	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(m_resourceManager, m_renderSystem, 1))
		return false;

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

	safeDestroy(m_primitiveRenderer);
	safeDestroy(m_resourceManager);
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

	AlignedVector< AnimatedMeshComponent::Binding > noBindings;

	Ref< AnimatedMeshComponent > meshComponent = new AnimatedMeshComponent(
		Transform::identity(),
		m_mesh,
		m_skeleton,
		m_poseController,
		jointRemap,
		noBindings,
		m_renderSystem,
		false
	);

	m_entity = new world::Entity();
	m_entity->setComponent(meshComponent);
}

void AnimationPreviewControl::updateWorldRenderer()
{
	safeDestroy(m_worldRenderer);

	ui::Size sz = getInnerRect().getSize();
	if (sz.cx <= 0 || sz.cy <= 0)
		return;

	Ref< world::WorldEntityRenderers > worldEntityRenderers = new world::WorldEntityRenderers();
	worldEntityRenderers->add(new world::EntityRenderer());
	worldEntityRenderers->add(new world::GroupRenderer());
	worldEntityRenderers->add(new world::LightRenderer());
	worldEntityRenderers->add(new mesh::MeshComponentRenderer());

	world::WorldRenderSettings wrs;
	wrs.viewNearZ = 0.1f;
	wrs.viewFarZ = 1000.0f;

	world::WorldCreateDesc wcd;
	wcd.worldRenderSettings = &wrs;
	wcd.entityRenderers = worldEntityRenderers;

	Ref< world::IWorldRenderer > worldRenderer = new world::WorldRendererForward();
	if (worldRenderer->create(
		m_resourceManager,
		m_renderSystem,
		wcd
	))
	{
		m_worldRenderer = worldRenderer;
		updateWorldRenderView();
	}
}

void AnimationPreviewControl::updateWorldRenderView()
{
	ui::Size sz = getInnerRect().getSize();
	m_worldRenderView.setPerspective(
		float(sz.cx),
		float(sz.cy),
		float(sz.cx) / sz.cy,
		deg2rad(65.0f),
		0.1f,
		1000.0f
	);
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

	if ((event->getKeyState() & ui::KsMenu) != 0)
	{
		if (event->getButton() == ui::MbtRight)
		{
			if ((event->getKeyState() & ui::KsControl) == 0)
			{
				// Move X/Z direction.
				float dx = -float(m_lastMousePosition.x - event->getPosition().x) * c_deltaMoveScale;
				float dz = -float(m_lastMousePosition.y - event->getPosition().y) * c_deltaMoveScale;
				m_position += Vector4(dx, 0.0f, dz, 0.0f);
			}
			else
			{
				// Move X/Y direction.
				float dx = -float(m_lastMousePosition.x - event->getPosition().x) * c_deltaMoveScale;
				float dy =  float(m_lastMousePosition.y - event->getPosition().y) * c_deltaMoveScale;
				m_position += Vector4(dx, dy, 0.0f, 0.0f);
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

void AnimationPreviewControl::eventSize(ui::SizeEvent* event)
{
	if (!m_renderView)
		return;

	ui::Size sz = event->getSize();
	m_renderView->reset(sz.cx, sz.cy);
}

void AnimationPreviewControl::eventPaint(ui::PaintEvent* event)
{
	if (!m_renderView)
		return;

	ui::Size sz = getInnerRect().getSize();
	float deltaTime = float(m_timer.getDeltaTime());
	float scaledTime = float(m_timer.getElapsedTime());

	float tmp[4];
	m_colorClear.getRGBA32F(tmp);
	Color4f clearColor(tmp[0], tmp[1], tmp[2], tmp[3]);

	float aspect = float(sz.cx) / sz.cy;

	Matrix44 viewTransform = translate(m_position) * rotateX(m_anglePitch) * rotateY(m_angleHead);
	Matrix44 projectionTransform = perspectiveLh(
		65.0f * PI / 180.0f,
		aspect,
		0.1f,
		1000.0f
	);

	Matrix44 viewInverse = viewTransform.inverse();
	Plane cameraPlane(
		viewInverse.axisZ(),
		viewInverse.translation()
	);

	// Setup world render passes.
	if (m_entity)
	{
		world::UpdateParams up;
		up.totalTime = scaledTime;
		up.deltaTime = deltaTime;
		up.alternateTime = scaledTime;
		m_entity->update(up);

		world::LightComponent lightComponent(
			world::LightType::LtDirectional,
			Color4f(1.0f, 1.0f, 1.0f, 1.0f),
			false,
			1000.0f,
			0.0f,
			0.0f,
			0.0f
		);

		world::Entity lightEntity;
		lightEntity.setComponent(&lightComponent);
		lightEntity.update(up);

		m_worldRenderView.setTimes(scaledTime, deltaTime, 1.0f);
		m_worldRenderView.setView(viewTransform, viewTransform);

		// \fixme
		//m_worldRenderer->attach(&lightEntity);
		//m_worldRenderer->attach(m_entity);
		//m_worldRenderer->setup(m_worldRenderView, *m_renderGraph, 0);
	}

	// Validate render graph.
	if (!m_renderGraph->validate())
		return;

	// Build render context.
	m_renderContext->flush();
	m_renderGraph->build(m_renderContext, sz.cx, sz.cy);

	// Render frame.
	render::Clear cl = {};
	cl.mask = render::CfColor | render::CfDepth;
	cl.colors[0] = clearColor;
	cl.depth = 1.0f;
	if (m_renderView->beginPass(&cl, render::TfAll, render::TfAll))
	{
		// Render context.
		m_renderContext->render(m_renderView);

		// Render guide lines.
		if (m_primitiveRenderer->begin(0, projectionTransform))
		{
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

			//if (m_entity && m_entity->getSkeleton())
			//{
			//	m_primitiveRenderer->pushDepthState(false, false, false);

			//	const resource::Proxy< Skeleton >& skeleton = m_entity->getSkeleton();

			//	AlignedVector< Transform > poseTransforms = m_entity->getPoseTransforms();
			//	if (poseTransforms.empty())
			//		calculateJointTransforms(skeleton, poseTransforms);

			//	if (poseTransforms.size() == skeleton->getJointCount())
			//	{
			//		for (uint32_t i = 0; i < skeleton->getJointCount(); ++i)
			//		{
			//			const Joint* joint = skeleton->getJoint(i);

			//			Color4ub color = Color4ub(255, 255, 0, 128);

			//			m_primitiveRenderer->drawWireFrame(poseTransforms[i].toMatrix44(), joint->getRadius() * 4.0f);

			//			if (joint->getParent() >= 0)
			//			{
			//				const Joint* parent = skeleton->getJoint(joint->getParent());
			//				T_ASSERT(parent);

			//				Vector4 start = poseTransforms[joint->getParent()].translation().xyz1();
			//				Vector4 end = poseTransforms[i].translation().xyz1();

			//				Vector4 z = (end - start).normalized();
			//				Vector4 y = cross(z, Vector4(0.0f, 1.0f, 0.0f, 0.0f));
			//				Vector4 x = cross(y, z);

			//				Scalar radius(parent->getRadius());
			//				x *= radius;
			//				y *= radius;
			//				z *= radius;

			//				m_primitiveRenderer->drawLine(start, start + z + x + y, color);
			//				m_primitiveRenderer->drawLine(start, start + z - x + y, color);
			//				m_primitiveRenderer->drawLine(start, start + z + x - y, color);
			//				m_primitiveRenderer->drawLine(start, start + z - x - y, color);

			//				m_primitiveRenderer->drawLine(start + z + x + y, end, color);
			//				m_primitiveRenderer->drawLine(start + z - x + y, end, color);
			//				m_primitiveRenderer->drawLine(start + z + x - y, end, color);
			//				m_primitiveRenderer->drawLine(start + z - x - y, end, color);

			//				m_primitiveRenderer->drawLine(start + z + x + y, start + z - x + y, color);
			//				m_primitiveRenderer->drawLine(start + z - x + y, start + z - x - y, color);
			//				m_primitiveRenderer->drawLine(start + z - x - y, start + z + x - y, color);
			//				m_primitiveRenderer->drawLine(start + z + x - y, start + z + x + y, color);
			//			}
			//		}
			//	}

			//	m_primitiveRenderer->popDepthState();
			//}

			m_primitiveRenderer->end(0);
			m_primitiveRenderer->render(m_renderView, 0);
		}

		m_renderView->endPass();
		m_renderView->present();
	}

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
}
