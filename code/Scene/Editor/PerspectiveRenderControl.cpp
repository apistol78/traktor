#include <limits>
#include "Scene/Editor/PerspectiveRenderControl.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/IEntityEditor.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/FrameEvent.h"
#include "Scene/Editor/SelectEvent.h"
#include "Editor/IEditor.h"
#include "Editor/Settings.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/PrimitiveRenderer.h"
#include "World/WorldRenderer.h"
#include "World/WorldRenderView.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldEntityRenderers.h"
#include "World/Entity/EntityInstance.h"
#include "World/Entity/Entity.h"
#include "Ui/MethodHandler.h"
#include "Ui/Widget.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/KeyEvent.h"
#include "Ui/Itf/IWidget.h"
#include "Core/Misc/EnterLeave.h"
#include "Core/Math/Vector2.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const float c_defaultFieldOfView = 45.0f;
const float c_minFieldOfView = 4.0f;
const float c_maxFieldOfView = 90.0f;
const float c_cameraTranslateDeltaScale = 0.025f;
const float c_cameraRotateDeltaScale = 0.01f;
const float c_deltaAdjust = 0.05f;
const float c_deltaAdjustSmall = 0.01f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.PerspectiveRenderControl", PerspectiveRenderControl, ui::Widget)

PerspectiveRenderControl::PerspectiveRenderControl()
:	m_fieldOfView(c_defaultFieldOfView)
,	m_mousePosition(0, 0)
,	m_mouseButton(0)
,	m_modifyCamera(false)
,	m_modifyAlternative(false)
,	m_dirtySize(0, 0)
{
}

bool PerspectiveRenderControl::create(ui::Widget* parent, SceneEditorContext* context)
{
	m_context = context;
	T_ASSERT (m_context);

	m_renderWidget = gc_new< ui::Widget >();
	if (!m_renderWidget->create(parent))
		return false;

	render::RenderViewCreateDesc desc;
	desc.depthBits = 24;
	desc.stencilBits = 0;
	desc.multiSample = 4;
	desc.waitVBlank = false;
	desc.mipBias = -1.0f;

	m_renderView = m_context->getRenderSystem()->createRenderView(m_renderWidget->getIWidget()->getSystemHandle(), desc);
	if (!m_renderView)
		return false;

	m_primitiveRenderer = gc_new< render::PrimitiveRenderer >();
	if (!m_primitiveRenderer->create(
		m_context->getResourceManager(),
		m_context->getRenderSystem()
	))
		return false;

	m_renderWidget->addButtonDownEventHandler(ui::createMethodHandler(this, &PerspectiveRenderControl::eventButtonDown));
	m_renderWidget->addButtonUpEventHandler(ui::createMethodHandler(this, &PerspectiveRenderControl::eventButtonUp));
	m_renderWidget->addMouseMoveEventHandler(ui::createMethodHandler(this, &PerspectiveRenderControl::eventMouseMove));
	m_renderWidget->addMouseWheelEventHandler(ui::createMethodHandler(this, &PerspectiveRenderControl::eventMouseWheel));
	m_renderWidget->addSizeEventHandler(ui::createMethodHandler(this, &PerspectiveRenderControl::eventSize));
	m_renderWidget->addPaintEventHandler(ui::createMethodHandler(this, &PerspectiveRenderControl::eventPaint));

	updateWorldRenderer();

	m_camera = gc_new< Camera >(cref(
		lookAt(Vector4(-4.0f, 4.0f, -4.0f, 1.0f), Vector4(0.0f, 0.0f, 0.0f, 1.0f))
	));
	m_timer.start();

	return true;
}

void PerspectiveRenderControl::destroy()
{
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

	if (m_renderWidget)
	{
		m_renderWidget->destroy();
		m_renderWidget = 0;
	}
}

void PerspectiveRenderControl::setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings)
{
	m_worldRenderSettings = worldRenderSettings;
	updateWorldRenderer();
}

bool PerspectiveRenderControl::handleCommand(const ui::Command& command)
{
	bool result = false;
	if (m_renderWidget->hasFocus())
	{
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
	}
	return result;
}

void PerspectiveRenderControl::update()
{
	m_renderWidget->update();
}

void PerspectiveRenderControl::updateWorldRenderer()
{
	if (!m_worldRenderSettings)
		return;

	if (m_worldRenderer)
	{
		m_worldRenderer->destroy();
		m_worldRenderer = 0;
	}

	ui::Size sz = m_renderWidget->getInnerRect().getSize();
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

	m_worldRenderer = gc_new< world::WorldRenderer >();
	if (m_worldRenderer->create(
		*m_worldRenderSettings,
		worldEntityRenderers,
		m_context->getRenderSystem(),
		m_renderView,
		4,
		1
	))
	{
		updateWorldRenderView();

		// Expose shadow map to debug view.
		Ref< render::RenderTargetSet > shadowTargetSet = m_worldRenderer->getShadowTargetSet();
		if (shadowTargetSet)
			m_context->setDebugTexture(shadowTargetSet->getColorTexture(0));
		else
			m_context->setDebugTexture(0);
	}
	else
		m_worldRenderer = 0;
}

void PerspectiveRenderControl::updateWorldRenderView()
{
	ui::Size sz = m_renderWidget->getInnerRect().getSize();

	world::WorldViewPerspective worldView;
	worldView.width = sz.cx;
	worldView.height = sz.cy;
	worldView.aspect = float(sz.cx) / sz.cy;
	worldView.fov = deg2rad(m_fieldOfView);
	m_worldRenderer->createRenderView(worldView, m_worldRenderView);
}

EntityAdapter* PerspectiveRenderControl::pickEntity(const ui::Point& position) const
{
	Frustum viewFrustum = m_worldRenderView.getViewFrustum();
	ui::Rect innerRect = m_renderWidget->getInnerRect();

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

void PerspectiveRenderControl::eventButtonDown(ui::Event* event)
{
	m_mousePosition = checked_type_cast< ui::MouseEvent* >(event)->getPosition();
	m_modifyCamera = (event->getKeyState() & ui::KsControl) == ui::KsControl;
	m_modifyAlternative = (event->getKeyState() & ui::KsMenu) == ui::KsMenu;

	// Are we already captured then we abort.
	if (m_renderWidget->hasCapture())
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

		m_renderWidget->setCapture();
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

	m_renderWidget->setFocus();
}

void PerspectiveRenderControl::eventButtonUp(ui::Event* event)
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

	if (m_renderWidget->hasCapture())
		m_renderWidget->releaseCapture();
}

void PerspectiveRenderControl::eventMouseMove(ui::Event* event)
{
	if (!m_renderWidget->hasCapture())
		return;

	int32_t mouseButton = (static_cast< ui::MouseEvent* >(event)->getButton() == ui::MouseEvent::BtLeft) ? 0 : 1;
	ui::Point mousePosition = checked_type_cast< ui::MouseEvent* >(event)->getPosition();

	Vector4 screenDelta(
		float(m_mousePosition.x - mousePosition.x),
		float(m_mousePosition.y - mousePosition.y),
		0.0f,
		0.0f
	);

	if (!m_modifyCamera)
	{
		// Apply modifier on selected entities.
		Ref< IModifier > modifier = m_context->getModifier();
		T_ASSERT (modifier);

		Matrix44 view = m_camera->getCurrentView();
		Matrix44 viewInverse = view.inverse();

		Matrix44 projection = m_worldRenderView.getProjection();
		Matrix44 projectionInverse = projection.inverse();

		ui::Rect innerRect = m_renderWidget->getInnerRect();
		Vector4 clipDelta = projectionInverse * (screenDelta * Vector4(-2.0f / innerRect.getWidth(), 2.0f / innerRect.getHeight(), 0.0f, 0.0f));

		for (RefArray< EntityAdapter >::iterator i = m_modifyEntities.begin(); i != m_modifyEntities.end(); ++i)
		{
			Ref< IEntityEditor > entityEditor = (*i)->getEntityEditor();
			if (entityEditor)
			{
				// Transform screen delta into world delta at entity's position.
				Vector4 viewPosition = view * (*i)->getTransform().translation();
				Vector4 viewDelta = clipDelta * viewPosition.z();
				Vector4 worldDelta = viewInverse * viewDelta;

				// Apply modifier through entity editor.
				entityEditor->applyModifier(
					m_context,
					*i,
					view,
					screenDelta,
					viewDelta,
					worldDelta,
					mouseButton
				);
			}
		}
	}
	else
	{
		if (mouseButton == 0)
		{
			screenDelta *= Scalar(c_cameraTranslateDeltaScale);
			if (m_modifyAlternative)
				m_camera->move(screenDelta.shuffle< 0, 1, 2, 3 >());
			else
				m_camera->move(screenDelta.shuffle< 0, 2, 1, 3 >());
		}
		else
		{
			screenDelta *= Scalar(c_cameraRotateDeltaScale);
			m_camera->rotate(screenDelta.y(), screenDelta.x());
		}
	}

	m_mousePosition = mousePosition;
	m_mouseButton = mouseButton;

	m_renderWidget->update();
}

void PerspectiveRenderControl::eventMouseWheel(ui::Event* event)
{
	int rotation = static_cast< ui::MouseEvent* >(event)->getWheelRotation();

	const float delta = 1.0f;

	if (m_context->getEditor()->getSettings()->getProperty(L"SceneEditor.InvertMouseWheel"))
		m_fieldOfView -= rotation * delta;
	else
		m_fieldOfView += rotation * delta;

	m_fieldOfView = max(m_fieldOfView, c_minFieldOfView);
	m_fieldOfView = min(m_fieldOfView, c_maxFieldOfView);

	updateWorldRenderView();
}

void PerspectiveRenderControl::eventSize(ui::Event* event)
{
	if (!m_renderView || !m_renderWidget->isVisible(true))
		return;

	ui::SizeEvent* s = static_cast< ui::SizeEvent* >(event);
	ui::Size sz = s->getSize();

	// Don't update world renderer if, in fact, size hasn't changed.
	if (sz.cx == m_dirtySize.cx && sz.cy == m_dirtySize.cy)
		return;

	m_renderView->resize(sz.cx, sz.cy);
	m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));

	updateWorldRenderer();

	m_dirtySize = sz;
}

void PerspectiveRenderControl::eventPaint(ui::Event* event)
{
	float deltaTime = float(m_timer.getDeltaTime());
	float scaledTime = m_context->getTime();

	// Update camera.
	m_camera->update(deltaTime);

	if (!m_renderView || !m_primitiveRenderer || !m_worldRenderer)
		return;

	// Get entities.
	RefArray< EntityAdapter > entityAdapters;
	m_context->getEntities(entityAdapters);

	// Get root entity.
	Ref< EntityAdapter > rootEntityAdapter = m_context->getRootEntityAdapter();
	Ref< world::Entity > rootEntity = rootEntityAdapter ? rootEntityAdapter->getEntity() : 0;

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

		Matrix44 view = m_camera->getCurrentView();

		m_primitiveRenderer->begin(m_renderView);
		m_primitiveRenderer->setClipDistance(m_worldRenderView.getViewFrustum().getNearZ());
		m_primitiveRenderer->pushProjection(m_worldRenderView.getProjection());
		m_primitiveRenderer->pushView(view);

		// Render XZ grid.
		const Color gridColor(0, 0, 0, 64);

		Vector4 viewPosition = view.inverse().translation();
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
				modifier->draw(
					m_context,
					view,
					(*i)->getTransform(),
					m_primitiveRenderer,
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
		{
			m_worldRenderer->build(m_worldRenderView, deltaTime, rootEntity, 0);
			m_worldRenderer->render(world::WrfDepthMap | world::WrfShadowMap | world::WrfVisualOpaque | world::WrfVisualAlphaBlend, 0);
			m_worldRenderer->flush(0);
		}

		m_primitiveRenderer->end(m_renderView);

		m_renderView->end();
		m_renderView->present();
	}

	event->consume();
}

	}
}
