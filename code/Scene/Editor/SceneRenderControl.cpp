#include <limits>
#include "Scene/Editor/SceneRenderControl.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/IEntityEditor.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/FrameEvent.h"
#include "Scene/Editor/SelectEvent.h"
#include "Scene/Scene.h"
#include "Scene/ISceneController.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/PrimitiveRenderer.h"
#include "Physics/PhysicsManager.h"
#include "World/WorldRenderer.h"
#include "World/WorldRenderView.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldEntityRenderers.h"
#include "World/PostProcess/PostProcess.h"
#include "World/Entity/EntityInstance.h"
#include "World/Entity/Entity.h"
#include "World/Entity/EntityUpdate.h"
#include "Ui/Application.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/KeyEvent.h"
#include "Ui/Events/IdleEvent.h"
#include "Ui/Itf/IWidget.h"
#include "Core/Misc/EnterLeave.h"
#include "Core/Math/Vector2.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const int c_updateInterval = 80;
const float c_cameraRotateDeltaScale = 0.01f;
const float c_deltaAdjust = 0.05f;
const float c_deltaAdjustSmall = 0.01f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SceneRenderControl", SceneRenderControl, ui::Widget)

SceneRenderControl::SceneRenderControl()
:	m_mousePosition(0, 0)
,	m_mouseButton(0)
,	m_modifyCamera(false)
,	m_modifyAlternative(false)
,	m_dirtySize(0, 0)
,	m_lastDeltaTime(1.0f / c_updateInterval)
,	m_lastPhysicsTime(0.0f)
{
}

bool SceneRenderControl::create(ui::Widget* parent, SceneEditorContext* context)
{
	m_context = context;
	T_ASSERT (m_context);

	if (!Widget::create(parent, ui::WsNone))
		return false;

	render::RenderViewCreateDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 0;
	desc.multiSample = 4;
	desc.waitVBlank = false;
	desc.mipBias = -1.0f;

	m_renderView = m_context->getRenderSystem()->createRenderView(getIWidget()->getSystemHandle(), desc);
	if (!m_renderView)
		return false;

	m_primitiveRenderer = gc_new< render::PrimitiveRenderer >();
	if (!m_primitiveRenderer->create(
		m_context->getResourceManager(),
		m_context->getRenderSystem()
	))
		return false;

	addButtonDownEventHandler(ui::createMethodHandler(this, &SceneRenderControl::eventButtonDown));
	addButtonUpEventHandler(ui::createMethodHandler(this, &SceneRenderControl::eventButtonUp));
	addMouseMoveEventHandler(ui::createMethodHandler(this, &SceneRenderControl::eventMouseMove));
	addMouseWheelEventHandler(ui::createMethodHandler(this, &SceneRenderControl::eventMouseWheel));
	addSizeEventHandler(ui::createMethodHandler(this, &SceneRenderControl::eventSize));
	addPaintEventHandler(ui::createMethodHandler(this, &SceneRenderControl::eventPaint));

	updateWorldRenderer();

	m_timer.start();

	// Register our event handler in case of message idle.
	m_idleHandler = ui::createMethodHandler(this, &SceneRenderControl::eventIdle);
	ui::Application::getInstance().addEventHandler(ui::EiIdle, m_idleHandler);

	return true;
}

void SceneRenderControl::destroy()
{
	if (m_idleHandler)
	{
		ui::Application::getInstance().removeEventHandler(ui::EiIdle, m_idleHandler);
		m_idleHandler = 0;
	}

	if (m_worldRenderer)
	{
		m_worldRenderer->destroy();
		m_worldRenderer = 0;
	}

	if (m_primitiveRenderer)
	{
		m_primitiveRenderer->destroy();
		m_primitiveRenderer = 0;
	}

	if (m_renderView)
	{
		m_renderView->close();
		m_renderView = 0;
	}

	Widget::destroy();
}

void SceneRenderControl::setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings)
{
	m_worldRenderSettings = worldRenderSettings;
	updateWorldRenderer();
}

void SceneRenderControl::setPostProcessSettings(world::PostProcessSettings* postProcessSettings)
{
	m_postProcessSettings = postProcessSettings;
	updatePostProcess();
}

bool SceneRenderControl::handleCommand(const ui::Command& command)
{
	bool result = false;

	RefArray< EntityAdapter > selectedEntities;
	m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);

	for (RefArray< EntityAdapter >::iterator i = selectedEntities.begin(); i != selectedEntities.end(); ++i)
	{
		Ref< IEntityEditor > entityEditor = (*i)->getEntityEditor();
		if (entityEditor)
		{
			// Propagate command to entity editor.
			result = entityEditor->handleCommand(m_context, *i, command);
			if (result)
				break;
		}
	}

	return result;
}

void SceneRenderControl::updateWorldRenderer()
{
	if (!m_worldRenderSettings)
		return;

	if (m_worldRenderer)
	{
		m_worldRenderer->destroy();
		m_worldRenderer = 0;
	}

	ui::Size sz = getInnerRect().getSize();
	if (sz.cx <= 0 || sz.cy <= 0)
		return;

	Ref< world::WorldEntityRenderers > worldEntityRenderers = gc_new< world::WorldEntityRenderers >();
	for (RefArray< ISceneEditorProfile >::const_iterator i = m_context->getEditorProfiles().begin(); i != m_context->getEditorProfiles().end(); ++i)
	{
		RefArray< world::IEntityRenderer > entityRenderers;
		(*i)->createEntityRenderers(m_context, m_renderView, m_primitiveRenderer, entityRenderers);

		for (RefArray< world::IEntityRenderer >::iterator j = entityRenderers.begin(); j != entityRenderers.end(); ++j)
			worldEntityRenderers->add(*j);
	}

	world::WorldViewPort worldViewPort;
	worldViewPort.width = sz.cx;
	worldViewPort.height = sz.cy;
	worldViewPort.aspect = float(sz.cx) / sz.cy;
	worldViewPort.fov = deg2rad(80.0f);

	m_worldRenderer = gc_new< world::WorldRenderer >();
	if (m_worldRenderer->create(
		*m_worldRenderSettings,
		worldEntityRenderers,
		m_context->getRenderSystem(),
		m_renderView,
		worldViewPort,
		4,
		1
	))
		m_worldRenderer->createRenderView(m_worldRenderView);
	else
		m_worldRenderer = 0;
}

void SceneRenderControl::updatePostProcess()
{
	if (m_postProcess)
	{
		m_postProcess->destroy();
		m_postProcess = 0;
	}

	if (m_renderTargetSet)
	{
		m_renderTargetSet->destroy();
		m_renderTargetSet = 0;
	}

	ui::Size sz = getInnerRect().getSize();

	if (m_postProcessSettings && sz.cx > 0 && sz.cy > 0)
	{
		m_postProcess = gc_new< world::PostProcess >();
		if (m_postProcess->create(
			m_postProcessSettings,
			m_context->getResourceManager(),
			m_context->getRenderSystem(),
			sz.cx,
			sz.cy
		))
		{
			render::RenderTargetSetCreateDesc desc;

			desc.count = 1;
			desc.width = sz.cx;
			desc.height = sz.cy;
			desc.multiSample = 4;
			desc.depthStencil = false;
			desc.targets[0].format = render::TfR8G8B8A8;

			m_renderTargetSet = m_context->getRenderSystem()->createRenderTargetSet(desc);
			if (!m_renderTargetSet)
			{
				m_postProcess->destroy();
				m_postProcess = 0;
			}
		}
		else
			m_postProcess = 0;
	}
	else
		m_postProcess = 0;
}

EntityAdapter* SceneRenderControl::pickEntity(const ui::Point& position) const
{
	Frustum viewFrustum = m_worldRenderView.getViewFrustum();
	ui::Rect innerRect = getInnerRect();

	Scalar fx(float(position.x) / innerRect.getWidth());
	Scalar fy(float(position.y) / innerRect.getHeight());

	// Interpolate frustum edges to find view pick-ray.
	const Vector4& viewEdgeTopLeft = viewFrustum.corners[4];
	const Vector4& viewEdgeTopRight = viewFrustum.corners[5];
	const Vector4& viewEdgeBottomLeft = viewFrustum.corners[7];
	const Vector4& viewEdgeBottomRight = viewFrustum.corners[6];

	Vector4 viewEdgeTop = lerp(viewEdgeTopLeft, viewEdgeTopRight, fx);
	Vector4 viewEdgeBottom = lerp(viewEdgeBottomLeft, viewEdgeBottomRight, fx);
	Vector4 viewRayDirection = lerp(viewEdgeTop, viewEdgeBottom, fy).normalized().xyz0();

	// Transform ray into world space.
	Matrix44 viewInv = m_worldRenderView.getView().inverseOrtho();
	Vector4 worldRayOrigin = viewInv.translation().xyz1();
	Vector4 worldRayDirection = viewInv * viewRayDirection;

	return m_context->queryRay(worldRayOrigin, worldRayDirection);
}

void SceneRenderControl::eventButtonDown(ui::Event* event)
{
	m_mousePosition = checked_type_cast< ui::MouseEvent* >(event)->getPosition();
	m_modifyCamera = (event->getKeyState() & ui::KsControl) == ui::KsControl;
	m_modifyAlternative = (event->getKeyState() & ui::KsMenu) == ui::KsMenu;

	// Are we already captured then we abort.
	if (hasCapture())
		return;

	if (m_modifyCamera || !m_context->inAddReferenceMode())
	{
		// Handle entity picking if enabled.
		if (!m_modifyCamera && m_context->getPickEnable())
		{
			Ref< EntityAdapter > entityAdapter = pickEntity(m_mousePosition);
			m_context->selectAllEntities(false);
			m_context->selectEntity(entityAdapter);
			m_context->raiseSelect();
		}

		if (!m_modifyCamera)
		{
			m_context->setPlaying(false);
			m_context->setPhysicsEnable(false);

			// Get selected entities.
			m_context->getEntities(
				m_modifyEntities,
				SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants
			);

			// Enter modify mode in entity editors.
			for (RefArray< EntityAdapter >::iterator i = m_modifyEntities.begin(); i != m_modifyEntities.end(); ++i)
			{
				if ((*i)->getEntityEditor())
					(*i)->getEntityEditor()->beginModifier(
						m_context,
						*i
					);
			}

			// Issue begin modification event.
			m_context->raisePreModify();
		}

		setCapture();
	}
	else
	{
		Ref< EntityAdapter > entityAdapter = pickEntity(m_mousePosition);
		if (entityAdapter)
		{
			// Get selected entities.
			m_context->getEntities(
				m_modifyEntities,
				SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants
			);

			if (!m_modifyEntities.empty())
			{
				// Issue begin modification event.
				m_context->raisePreModify();

				// Add reference to all selected entities.
				bool referenceAdded = false;
				for (RefArray< EntityAdapter >::iterator i = m_modifyEntities.begin(); i != m_modifyEntities.end(); ++i)
				{
					if ((*i) != entityAdapter)
						referenceAdded |= (*i)->addReference(entityAdapter);
				}

				// Need to build entities so references in them will get updated.
				if (referenceAdded)
					m_context->buildEntities();

				// Issue post modification event.
				m_context->raisePostModify();
			}
		}
	}

	setFocus();
}

void SceneRenderControl::eventButtonUp(ui::Event* event)
{
	// Issue finished modification event.
	if (!m_modifyCamera)
	{
		m_context->raisePostModify();

		// Leave modify mode in entity editors.
		for (RefArray< EntityAdapter >::iterator i = m_modifyEntities.begin(); i != m_modifyEntities.end(); ++i)
		{
			if ((*i)->getEntityEditor())
				(*i)->getEntityEditor()->endModifier(
				m_context,
				*i
			);
		}
	}

	m_modifyEntities.resize(0);
	m_modifyCamera = false;
	m_modifyAlternative = false;

	if (hasCapture())
		releaseCapture();
}

void SceneRenderControl::eventMouseMove(ui::Event* event)
{
	if (!hasCapture())
		return;

	Ref< Camera > camera = m_context->getCamera();
	T_ASSERT (camera);

	int mouseButton = (static_cast< ui::MouseEvent* >(event)->getButton() == ui::MouseEvent::BtLeft) ? 0 : 1;
	ui::Point mousePosition = checked_type_cast< ui::MouseEvent* >(event)->getPosition();

	Vector2 mouseDelta(
		float(m_mousePosition.x - mousePosition.x),
		float(m_mousePosition.y - mousePosition.y)
	);

	if (!m_modifyCamera)
	{
		// Apply modifier on selected entities.
		Ref< IModifier > modifier = m_context->getModifier();
		T_ASSERT (modifier);

		for (RefArray< EntityAdapter >::iterator i = m_modifyEntities.begin(); i != m_modifyEntities.end(); ++i)
		{
			Ref< IEntityEditor > entityEditor = (*i)->getEntityEditor();
			if (entityEditor)
				entityEditor->applyModifier(
					m_context,
					*i,
					camera->getCurrentView(),
					mouseDelta,
					mouseButton
				);
		}
	}
	else
	{
		if (mouseButton == 0)
		{
			mouseDelta *= m_context->getDeltaScale();
			if (m_modifyAlternative)
				camera->move(Vector4(mouseDelta.x, mouseDelta.y, 0.0f, 0.0f));
			else
				camera->move(Vector4(mouseDelta.x, 0.0f, mouseDelta.y, 0.0f));
		}
		else
		{
			mouseDelta *= c_cameraRotateDeltaScale;
			camera->rotate(mouseDelta.y, mouseDelta.x);
		}
	}

	m_mousePosition = mousePosition;
	m_mouseButton = mouseButton;

	update();
}

void SceneRenderControl::eventMouseWheel(ui::Event* event)
{
	int rotation = static_cast< ui::MouseEvent* >(event)->getWheelRotation();
	
	float deltaScale = m_context->getDeltaScale();

	if (rotation < 0)
	{
		if (deltaScale <= c_deltaAdjust)
			deltaScale -= c_deltaAdjustSmall;
		else
			deltaScale -= c_deltaAdjust;
	}
	else if (rotation > 0)
	{
		if (deltaScale < c_deltaAdjust - std::numeric_limits< float >::epsilon())
			deltaScale += c_deltaAdjustSmall;
		else
			deltaScale += c_deltaAdjust;
	}

	deltaScale = std::max(deltaScale, c_deltaAdjustSmall);
	m_context->setDeltaScale(deltaScale);
}

void SceneRenderControl::eventSize(ui::Event* event)
{
	if (!m_renderView || !isVisible(true))
		return;

	ui::SizeEvent* s = static_cast< ui::SizeEvent* >(event);
	ui::Size sz = s->getSize();

	// Don't update world renderer if, in fact, size hasn't changed.
	if (sz.cx == m_dirtySize.cx && sz.cy == m_dirtySize.cy)
		return;

	m_renderView->resize(sz.cx, sz.cy);
	m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));

	updateWorldRenderer();
	updatePostProcess();

	m_dirtySize = sz;
}

void SceneRenderControl::eventPaint(ui::Event* event)
{
	double startTime = m_timer.getElapsedTime();

	// Filter delta time.
	float deltaTime = float(m_timer.getDeltaTime());
	deltaTime = std::min(deltaTime, 1.0f / 10.0f);
	deltaTime = float(deltaTime * 0.2f + m_lastDeltaTime * 0.8f);
	m_lastDeltaTime = deltaTime;

	float scaledTime = m_context->getTime();
	float scaledDeltaTime = m_context->isPlaying() ? deltaTime * m_context->getTimeScale() : 0.0f;

	// Update camera.
	Ref< Camera > camera = m_context->getCamera();
	T_ASSERT (camera);

	camera->update(deltaTime);

	if (!m_renderView || !m_primitiveRenderer || !m_worldRenderer)
		return;

	// Update physics; update in steps of 1/60th of a second.
	if (m_context->getPhysicsEnable())
	{
		while (m_lastPhysicsTime < scaledTime)
		{
			m_context->getPhysicsManager()->update();
			m_lastPhysicsTime += 1.0f / 60.0f;
		}
	}

	// Update controller.
	Ref< Scene > scene = m_context->getScene();
	if (scene)
	{
		Ref< ISceneController > controller = scene->getController();
		if (controller)
		{
			controller->update(
				m_context->getScene(),
				scaledTime,
				scaledDeltaTime
			);
		}
	}

	// Get entities.
	RefArray< EntityAdapter > entityAdapters;
	m_context->getEntities(entityAdapters);

	// Get root entity.
	Ref< EntityAdapter > rootEntityAdapter = m_context->getRootEntityAdapter();
	Ref< world::Entity > rootEntity = rootEntityAdapter ? rootEntityAdapter->getEntity() : 0;

	// Update entities.
	if (rootEntity)
	{
		world::EntityUpdate entityUpdate(scaledDeltaTime);
		rootEntity->update(&entityUpdate);
	}

	// Render world.
	if (m_renderView->begin())
	{
		const float clearColor[] = { 0.7f, 0.7f, 0.7f, 0.0f };
		m_renderView->clear(
			render::CfColor | render::CfDepth,
			clearColor,
			1.0f,
			128
		);

		Matrix44 view = camera->getCurrentView();

		m_primitiveRenderer->begin(m_renderView);
		m_primitiveRenderer->setClipDistance(m_worldRenderView.getViewFrustum().getNearZ());
		m_primitiveRenderer->pushProjection(m_worldRenderView.getProjection());
		m_primitiveRenderer->pushView(view);

		// Render XZ grid.
		const Color gridColor(0, 0, 0, 64);

		Vector4 viewPosition = view.inverseOrtho().translation();
		float vx = floorf(viewPosition.x());
		float vz = floorf(viewPosition.z());

		for (int x = -20; x <= 20; ++x)
		{
			float fx = float(x);
			m_primitiveRenderer->drawLine(
				Vector4(fx + vx, 0.0f, -20.0f + vz, 1.0f),
				Vector4(fx + vx, 0.0f, 20.0f + vz, 1.0f),
				(int(fx + vx) == 0) ? 2.0f : 0.0f,
				gridColor
			);
			m_primitiveRenderer->drawLine(
				Vector4(-20.0f + vx, 0.0f, fx + vz, 1.0f),
				Vector4(20.0f + vx, 0.0f, fx + vz, 1.0f),
				(int(fx + vz) == 0) ? 2.0f : 0.0f,
				gridColor
			);
		}

		// Draw selection marker(s).
		Ref< IModifier > modifier = m_context->getModifier();
		for (RefArray< EntityAdapter >::const_iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
		{
			// Draw modifier and reference arrows; only applicable to spatial entities we must first check if it's a spatial entity.
			if (modifier && (*i)->isSpatial() && (*i)->isSelected() && !(*i)->isChildOfExternal())
			{
				bool modifierActive = (event->getKeyState() & ui::KsControl) == 0 && hasCapture();
				modifier->draw(
					m_context,
					(*i)->getTransform(),
					m_primitiveRenderer,
					modifierActive,
					m_mouseButton
				);

				// Draw reference arrows.
				const RefArray< world::EntityInstance >& references = (*i)->getInstance()->getReferences();
				for (RefArray< world::EntityInstance >::const_iterator j = references.begin(); j != references.end(); ++j)
				{
					Ref< EntityAdapter > referencedAdapter = m_context->findAdapterFromInstance(*j);
					if (referencedAdapter)
					{
						Vector4 sourcePosition = (*i)->getTransform().translation();
						Vector4 targetPosition = referencedAdapter->getTransform().translation();

						Vector4 center = (sourcePosition + targetPosition) / Scalar(2.0f);
						Vector4 offset = (targetPosition - sourcePosition) / Scalar(20.0f);

						Scalar length = offset.length();
						if (length > 0.1f)
							offset *= Scalar(0.1f) / length;

						m_primitiveRenderer->drawLine(sourcePosition, targetPosition, 3.0f, Color(255, 200, 0, 255));
						m_primitiveRenderer->drawArrowHead(center - offset, center + offset, 0.5f, Color(255, 200, 0, 255));
					}
				}
			}

			// Draw entity guides.
			m_context->drawGuide(m_primitiveRenderer, *i);
		}

		// Render entities.
		m_worldRenderView.setView(view);

		if (rootEntity)
			m_worldRenderer->build(m_worldRenderView, scaledDeltaTime, rootEntity, 0);

		// Flush rendering queue to GPU.
		double startRenderTime = m_timer.getElapsedTime();

		if (rootEntity)
			m_worldRenderer->render(world::WrfDepthMap | world::WrfShadowMap, 0);

		if (m_postProcess)
		{
			m_renderView->begin(m_renderTargetSet, 0, true);
			m_renderView->clear(render::CfColor, clearColor, 1.0f, 128);
		}

		if (rootEntity)
		{
			m_worldRenderer->render(world::WrfVisualOpaque | world::WrfVisualAlphaBlend, 0);
			m_worldRenderer->flush(0);
		}

		if (m_postProcess)
		{
			m_renderView->end();
			m_postProcess->render(
				m_worldRenderView,
				m_renderView,
				m_renderTargetSet,
				m_worldRenderer->getDepthTargetSet(),
				deltaTime
			);
		}

		m_primitiveRenderer->end(m_renderView);

		m_renderView->end();
		m_renderView->present();

		double stopTime = m_timer.getElapsedTime();

		// Notify frame handlers about frame time.
		if ((int32_t(stopTime * 100.0) & 15) == 0)
		{
			FrameEvent eventFrame(this, stopTime - startTime, stopTime - startRenderTime);
			m_context->raisePostFrame(&eventFrame);
		}
	}

	// Update context time.
	m_context->setTime(scaledTime + scaledDeltaTime);

	event->consume();
}

void SceneRenderControl::eventIdle(ui::Event* event)
{
	ui::IdleEvent* idleEvent = checked_type_cast< ui::IdleEvent* >(event);
	if (isVisible(true))
	{
		update();
		idleEvent->requestMore();
	}
}

	}
}
