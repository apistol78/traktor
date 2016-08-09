#include "Animation/AnimatedMeshEntity.h"
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/Animation/AnimationFactory.h"
#include "Animation/Editor/AnimationPreviewControl.h"
#include "Core/Math/Plane.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/IEditor.h"
#include "Mesh/MeshComponentRenderer.h"
#include "Mesh/MeshEntityRenderer.h"
#include "Mesh/MeshFactory.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Render/Context/RenderContext.h"
#include "Render/ImageProcess/ImageProcessFactory.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/TextureFactory.h"
#include "Resource/ResourceManager.h"
#include "Ui/Application.h"
#include "Ui/Itf/IWidget.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldRenderSettings.h"
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
	if (!Widget::create(parent, ui::WsNone))
		return false;

	m_renderSystem = m_editor->getStoreObject< render::IRenderSystem >(L"RenderSystem");
	if (!m_renderSystem)
		return false;

	Ref< db::Database > resourceDatabase = m_editor->getOutputDatabase();
	if (!resourceDatabase)
		return false;

	m_resourceManager = new resource::ResourceManager(true);
	m_resourceManager->addFactory(new AnimationFactory(resourceDatabase));
	m_resourceManager->addFactory(new mesh::MeshFactory(resourceDatabase, m_renderSystem));
	m_resourceManager->addFactory(new render::ShaderFactory(resourceDatabase, m_renderSystem));
	m_resourceManager->addFactory(new render::TextureFactory(resourceDatabase, m_renderSystem, 0));
	m_resourceManager->addFactory(new render::ImageProcessFactory(resourceDatabase));

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 24;
	desc.stencilBits = 0;
	desc.multiSample = 0;
	desc.waitVBlanks = 0;
	desc.syswin = getIWidget()->getSystemWindow();

	m_renderView = m_renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(m_resourceManager, m_renderSystem))
		return false;

	addEventHandler< ui::MouseButtonDownEvent >(this, &AnimationPreviewControl::eventButtonDown);
	addEventHandler< ui::MouseButtonUpEvent >(this, &AnimationPreviewControl::eventButtonUp);
	addEventHandler< ui::MouseMoveEvent >(this, &AnimationPreviewControl::eventMouseMove);
	addEventHandler< ui::SizeEvent >(this, &AnimationPreviewControl::eventSize);
	addEventHandler< ui::PaintEvent >(this, &AnimationPreviewControl::eventPaint);

	updateSettings();
	updateWorldRenderer();

	m_idleEventHandler = ui::Application::getInstance()->addEventHandler< ui::IdleEvent >(this, &AnimationPreviewControl::eventIdle);

	m_timer.start();
	return true;
}

void AnimationPreviewControl::destroy()
{
	ui::Application::getInstance()->removeEventHandler< ui::IdleEvent >(m_idleEventHandler);

	safeDestroy(m_primitiveRenderer);
	safeDestroy(m_resourceManager);

	if (m_renderView)
	{
		m_renderView->close();
		m_renderView = 0;
	}

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
	m_colorClear = colors->getProperty< PropertyColor >(L"Background");
	m_colorGrid = colors->getProperty< PropertyColor >(L"Grid");
}

void AnimationPreviewControl::updatePreview()
{
	m_entity = 0;

	if (!m_mesh)
		return;

	std::vector< int32_t > jointRemap;
	if (m_skeleton)
	{
		jointRemap.resize(m_skeleton->getJointCount());

		const std::map< std::wstring, int32_t >& jointMap = m_mesh->getJointMap();
		for (uint32_t i = 0; i < m_skeleton->getJointCount(); ++i)
		{
			const Joint* joint = m_skeleton->getJoint(i);
			T_ASSERT (joint);

			std::map< std::wstring, int32_t >::const_iterator j = jointMap.find(joint->getName());
			if (j == jointMap.end())
			{
				jointRemap[i] = -1;
				continue;
			}

			jointRemap[i] = j->second;
		}
	}

	std::vector< AnimatedMeshEntity::Binding > noBindings;
	m_entity = new AnimatedMeshEntity(
		Transform::identity(),
		m_mesh,
		m_skeleton,
		m_poseController,
		jointRemap,
		noBindings,
		false,
		false,
		false
	);
}

void AnimationPreviewControl::updateWorldRenderer()
{
	safeDestroy(m_worldRenderer);

	ui::Size sz = getInnerRect().getSize();
	if (sz.cx <= 0 || sz.cy <= 0)
		return;

	Ref< world::WorldEntityRenderers > worldEntityRenderers = new world::WorldEntityRenderers();
	worldEntityRenderers->add(new mesh::MeshComponentRenderer());
	worldEntityRenderers->add(new mesh::MeshEntityRenderer());

	world::WorldRenderSettings wrs;
	wrs.viewNearZ = 0.1f;
	wrs.viewFarZ = 1000.0f;

	world::WorldCreateDesc wcd;
	wcd.worldRenderSettings = &wrs;
	wcd.entityRenderers = worldEntityRenderers;
	wcd.frameCount = 1;

	Ref< world::IWorldRenderer > worldRenderer = new world::WorldRendererForward();
	if (worldRenderer->create(
		m_resourceManager,
		m_renderSystem,
		m_renderView,
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
		sz.cx,
		sz.cy,
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
	m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));

	updateWorldRenderer();
}

void AnimationPreviewControl::eventPaint(ui::PaintEvent* event)
{
	float deltaTime = float(m_timer.getDeltaTime());
	float scaledTime = float(m_timer.getElapsedTime());

	if (!m_renderView || !m_renderView->begin(render::EtCyclop))
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

	m_worldRenderView.setTimes(scaledTime, deltaTime, 1.0f);
	m_worldRenderView.setView(viewTransform);

	if (m_entity)
	{
		world::UpdateParams up;
		up.totalTime = scaledTime;
		up.deltaTime = deltaTime;
		up.alternateTime = scaledTime;
		m_entity->update(up);

		world::Light light;
		light.type = world::LtDirectional;
		light.position = Vector4::origo();
		light.direction = Vector4(0.0f, 1.0f, 0.0f);
		light.sunColor = Vector4(1.0f, 1.0f, 1.0f);
		light.baseColor = Vector4(0.6f, 0.6f, 0.6f);
		light.shadowColor = Vector4(0.3f, 0.3f, 0.3f);
		light.range = Scalar(0.0f);
		light.radius = Scalar(0.0f);
		light.castShadow = false;

		m_worldRenderView.resetLights();
		m_worldRenderView.addLight(light);

		m_worldRenderer->beginBuild();
		m_worldRenderer->build(m_entity);
		m_worldRenderer->endBuild(m_worldRenderView, 0);
	}

	m_worldRenderer->beginRender(
		0,
		render::EtCyclop,
		clearColor
	);

	m_worldRenderer->render(
		world::WrfDepthMap | world::WrfNormalMap | world::WrfShadowMap | world::WrfLightMap | world::WrfVisualOpaque | world::WrfVisualAlphaBlend,
		0,
		render::EtCyclop
	);

	m_worldRenderer->endRender(0, render::EtCyclop, deltaTime);

	if (m_primitiveRenderer->begin(m_renderView, projectionTransform))
	{
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

		if (m_entity && m_entity->getSkeleton())
		{
			m_primitiveRenderer->pushDepthState(false, false, false);

			const resource::Proxy< Skeleton >& skeleton = m_entity->getSkeleton();

			AlignedVector< Transform > poseTransforms = m_entity->getPoseTransforms();
			if (poseTransforms.empty())
				calculateJointTransforms(skeleton, poseTransforms);

			if (poseTransforms.size() == skeleton->getJointCount())
			{
				for (uint32_t i = 0; i < skeleton->getJointCount(); ++i)
				{
					const Joint* joint = skeleton->getJoint(i);

					Color4ub color = Color4ub(255, 255, 0, 128);

					m_primitiveRenderer->drawWireFrame(poseTransforms[i].toMatrix44(), joint->getRadius() * 4.0f);

					if (joint->getParent() >= 0)
					{
						const Joint* parent = skeleton->getJoint(joint->getParent());
						T_ASSERT (parent);

						Vector4 start = poseTransforms[joint->getParent()].translation().xyz1();
						Vector4 end = poseTransforms[i].translation().xyz1();

						Vector4 z = (end - start).normalized();
						Vector4 y = cross(z, Vector4(0.0f, 1.0f, 0.0f, 0.0f));
						Vector4 x = cross(y, z);

						Scalar radius(parent->getRadius());
						x *= radius;
						y *= radius;
						z *= radius;

						m_primitiveRenderer->drawLine(start, start + z + x + y, color);
						m_primitiveRenderer->drawLine(start, start + z - x + y, color);
						m_primitiveRenderer->drawLine(start, start + z + x - y, color);
						m_primitiveRenderer->drawLine(start, start + z - x - y, color);

						m_primitiveRenderer->drawLine(start + z + x + y, end, color);
						m_primitiveRenderer->drawLine(start + z - x + y, end, color);
						m_primitiveRenderer->drawLine(start + z + x - y, end, color);
						m_primitiveRenderer->drawLine(start + z - x - y, end, color);

						m_primitiveRenderer->drawLine(start + z + x + y, start + z - x + y, color);
						m_primitiveRenderer->drawLine(start + z - x + y, start + z - x - y, color);
						m_primitiveRenderer->drawLine(start + z - x - y, start + z + x - y, color);
						m_primitiveRenderer->drawLine(start + z + x - y, start + z + x + y, color);
					}
				}
			}

			m_primitiveRenderer->popDepthState();
		}

		m_primitiveRenderer->end();
	}

	m_renderView->end();
	m_renderView->present();

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
