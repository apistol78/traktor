#include <limits>
#include "Scene/Editor/OrthogonalRenderControl.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/IEntityEditor.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/CameraMesh.h"
#include "Scene/Scene.h"
#include "Editor/IEditor.h"
#include "Editor/Settings.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "World/WorldRenderer.h"
#include "World/WorldRenderView.h"
#include "World/WorldEntityRenderers.h"
#include "World/Entity/EntityInstance.h"
#include "Ui/MethodHandler.h"
#include "Ui/Widget.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/KeyEvent.h"
#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const float c_cameraTranslateDeltaScale = 0.025f;
const float c_minMagnification = 0.01f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.OrthogonalRenderControl", OrthogonalRenderControl, Object)

OrthogonalRenderControl::OrthogonalRenderControl()
:	m_mousePosition(0, 0)
,	m_mouseButton(0)
,	m_modifyCamera(false)
,	m_modifyAlternative(false)
,	m_modifyBegun(false)
,	m_viewPlane(PositiveX)
,	m_viewFarZ(0.0f)
,	m_magnification(10.0f)
,	m_cameraX(0.0f)
,	m_cameraY(0.0f)
,	m_dirtySize(0, 0)
{
}

bool OrthogonalRenderControl::create(ui::Widget* parent, SceneEditorContext* context, ViewPlane viewPlane)
{
	m_context = context;
	T_ASSERT (m_context);

	m_viewPlane = viewPlane;

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

	m_renderWidget->addButtonDownEventHandler(ui::createMethodHandler(this, &OrthogonalRenderControl::eventButtonDown));
	m_renderWidget->addButtonUpEventHandler(ui::createMethodHandler(this, &OrthogonalRenderControl::eventButtonUp));
	m_renderWidget->addMouseMoveEventHandler(ui::createMethodHandler(this, &OrthogonalRenderControl::eventMouseMove));
	m_renderWidget->addMouseWheelEventHandler(ui::createMethodHandler(this, &OrthogonalRenderControl::eventMouseWheel));
	m_renderWidget->addSizeEventHandler(ui::createMethodHandler(this, &OrthogonalRenderControl::eventSize));
	m_renderWidget->addPaintEventHandler(ui::createMethodHandler(this, &OrthogonalRenderControl::eventPaint));

	updateWorldRenderer();

	m_timer.start();

	return true;
}

void OrthogonalRenderControl::destroy()
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

void OrthogonalRenderControl::updateWorldRenderer()
{
	if (m_worldRenderer)
	{
		m_worldRenderer->destroy();
		m_worldRenderer = 0;
	}

	Ref< scene::Scene > sceneInstance = m_context->getScene();
	if (!sceneInstance)
		return;

	ui::Size sz = m_renderWidget->getInnerRect().getSize();
	if (sz.cx <= 0 || sz.cy <= 0)
		return;

	Ref< const world::WorldRenderSettings > worldRenderSettings = sceneInstance->getWorldRenderSettings();

	// Create entity renderers.
	Ref< world::WorldEntityRenderers > worldEntityRenderers = gc_new< world::WorldEntityRenderers >();
	for (RefArray< ISceneEditorProfile >::const_iterator i = m_context->getEditorProfiles().begin(); i != m_context->getEditorProfiles().end(); ++i)
	{
		RefArray< world::IEntityRenderer > entityRenderers;
		(*i)->createEntityRenderers(m_context, m_renderView, m_primitiveRenderer, entityRenderers);
		for (RefArray< world::IEntityRenderer >::iterator j = entityRenderers.begin(); j != entityRenderers.end(); ++j)
			worldEntityRenderers->add(*j);
	}

	// Create a copy of the render settings; we don't want to enable shadows nor velocity in this view.
	world::WorldRenderSettings wrs = *worldRenderSettings;
	wrs.depthPassEnabled = false;
	wrs.velocityPassEnable = false;
	wrs.shadowsEnabled = false;

	m_worldRenderer = gc_new< world::WorldRenderer >();
	if (!m_worldRenderer->create(
		&wrs,
		worldEntityRenderers,
		m_context->getResourceManager(),
		m_context->getRenderSystem(),
		m_renderView,
		4,
		1
	))
		m_worldRenderer = 0;

	m_viewFarZ = wrs.viewFarZ;
}

bool OrthogonalRenderControl::handleCommand(const ui::Command& command)
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

void OrthogonalRenderControl::update()
{
	m_renderWidget->update();
}

Matrix44 OrthogonalRenderControl::getProjectionTransform() const
{
	ui::Rect innerRect = m_renderWidget->getInnerRect();
	float ratio = float(innerRect.getWidth()) / innerRect.getHeight();

	world::WorldViewOrtho worldView;
	worldView.width = m_magnification;
	worldView.height = m_magnification / ratio;

	world::WorldRenderView worldRenderView;
	m_worldRenderer->createRenderView(worldView, worldRenderView);

	return worldRenderView.getProjection();
}

Matrix44 OrthogonalRenderControl::getViewTransform() const
{
	switch (m_viewPlane)
	{
	case PositiveX:
		return translate(m_cameraX, m_cameraY, 0.0f) * rotateY(deg2rad(-90.0f));
	case NegativeX:
		return translate(m_cameraX, m_cameraY, 0.0f) * rotateY(deg2rad(90.0f));
	case PositiveY:
		return translate(m_cameraX, m_cameraY, 0.0f) * rotateX(deg2rad(-90.0f));
	case NegativeY:
		return translate(m_cameraX, m_cameraY, 0.0f) * rotateX(deg2rad(90.0f));
	case PositiveZ:
		return translate(m_cameraX, m_cameraY, 0.0f);
	case NegativeZ:
		return translate(m_cameraX, m_cameraY, 0.0f) * rotateY(deg2rad(180.0f));
	}
	return Matrix44::identity();
}

EntityAdapter* OrthogonalRenderControl::pickEntity(const ui::Point& position) const
{
	const float c_viewFarOffset = 1.0f;
	const ui::Rect innerRect = m_renderWidget->getInnerRect();

	Matrix44 projection = getProjectionTransform();
	Matrix44 projectionInverse = projection.inverse();

	Matrix44 view = getViewTransform();
	Matrix44 viewInverse = view.inverse();

	Scalar fx( float(position.x * 2.0f) / innerRect.getWidth() - 1.0f);
	Scalar fy(-float(position.y * 2.0f) / innerRect.getHeight() + 1.0f);

	Vector4 clipPosition(fx, fy, 0.0f, 1.0f);
	Vector4 viewPosition = projectionInverse * clipPosition + Vector4(0.0f, 0.0f, -(m_viewFarZ - c_viewFarOffset));
	Vector4 worldRayOrigin = viewInverse * viewPosition;
	Vector4 worldRayDirection = viewInverse.axisZ();

	return m_context->queryRay(worldRayOrigin, worldRayDirection);
}

void OrthogonalRenderControl::eventButtonDown(ui::Event* event)
{
	m_mousePosition = checked_type_cast< ui::MouseEvent* >(event)->getPosition();
	m_modifyCamera = (event->getKeyState() & ui::KsControl) == ui::KsControl;
	m_modifyAlternative = (event->getKeyState() & ui::KsMenu) == ui::KsMenu;

	// Are we already captured then we abort.
	if (m_renderWidget->hasCapture())
		return;

	if (m_modifyCamera)
	{
		m_renderWidget->setCapture();
	}
	else  if (!m_context->inAddReferenceMode())
	{
		// Handle entity picking if enabled.
		if (m_context->getPickEnable())
		{
			Ref< EntityAdapter > entityAdapter = pickEntity(m_mousePosition);

			// De-select all other if shift isn't held.
			if ((event->getKeyState() & ui::KsShift) == 0)
				m_context->selectAllEntities(false);

			m_context->selectEntity(entityAdapter);
			m_context->raiseSelect(this);
		}

		m_context->setPlaying(false);
		m_context->setPhysicsEnable(false);

		m_modifyBegun = false;
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

void OrthogonalRenderControl::eventButtonUp(ui::Event* event)
{
	// Issue finished modification event.
	if (!m_modifyCamera)
	{
		if (m_modifyBegun)
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
	}

	m_modifyEntities.resize(0);
	m_modifyCamera = false;
	m_modifyAlternative = false;
	m_modifyBegun = false;

	if (m_renderWidget->hasCapture())
		m_renderWidget->releaseCapture();
}

void OrthogonalRenderControl::eventMouseMove(ui::Event* event)
{
	if (!m_renderWidget->hasCapture())
		return;

	int mouseButton = (static_cast< ui::MouseEvent* >(event)->getButton() == ui::MouseEvent::BtLeft) ? 0 : 1;
	ui::Point mousePosition = checked_type_cast< ui::MouseEvent* >(event)->getPosition();

	Vector4 screenDelta(
		float(m_mousePosition.x - mousePosition.x),
		float(m_mousePosition.y - mousePosition.y),
		0.0f,
		0.0f
	);

	if (!m_modifyCamera)
	{
		if (!m_modifyBegun)
		{
			// Clone selection set.
			if (m_modifyAlternative)
				m_context->cloneSelected();

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

			m_modifyBegun = true;
		}

		// Apply modifier on selected entities.
		Ref< IModifier > modifier = m_context->getModifier();
		T_ASSERT (modifier);

		Matrix44 projection = getProjectionTransform();
		Matrix44 projectionInverse = projection.inverse();

		Matrix44 view = getViewTransform();
		Matrix44 viewInverse = view.inverse();

		ui::Rect innerRect = m_renderWidget->getInnerRect();
		Vector4 clipDelta = projectionInverse * (screenDelta * Vector4(-2.0f / innerRect.getWidth(), 2.0f / innerRect.getHeight(), 0.0f, 0.0f));

		for (RefArray< EntityAdapter >::iterator i = m_modifyEntities.begin(); i != m_modifyEntities.end(); ++i)
		{
			Ref< IEntityEditor > entityEditor = (*i)->getEntityEditor();
			if (entityEditor)
			{
				// Transform screen delta into world delta at entity's position.
				Vector4 viewDelta = clipDelta;
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
		Matrix44 projection = getProjectionTransform();
		Matrix44 projectionInverse = projection.inverse();

		ui::Rect innerRect = m_renderWidget->getInnerRect();
		Vector4 clipDelta = projectionInverse * (screenDelta * Vector4(-2.0f / innerRect.getWidth(), 2.0f / innerRect.getHeight(), 0.0f, 0.0f));

		m_cameraX += clipDelta.x();
		m_cameraY += clipDelta.y();
	}

	m_mousePosition = mousePosition;
	m_mouseButton = mouseButton;

	m_renderWidget->update();
}

void OrthogonalRenderControl::eventMouseWheel(ui::Event* event)
{
	int rotation = static_cast< ui::MouseEvent* >(event)->getWheelRotation();
	
	float delta = m_magnification / 10.0f;

	if (m_context->getEditor()->getSettings()->getProperty(L"SceneEditor.InvertMouseWheel"))
		m_magnification -= rotation * delta;
	else
		m_magnification += rotation * delta;

	if (m_magnification < c_minMagnification)
		m_magnification = c_minMagnification;
}

void OrthogonalRenderControl::eventSize(ui::Event* event)
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

void OrthogonalRenderControl::eventPaint(ui::Event* event)
{
	float deltaTime = float(m_timer.getDeltaTime());
	float scaledTime = m_context->getTime();

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

		float ratio = float(m_dirtySize.cx) / m_dirtySize.cy;

		world::WorldViewOrtho worldView;
		worldView.width = m_magnification;
		worldView.height = m_magnification / ratio;

		world::WorldRenderView worldRenderView;
		m_worldRenderer->createRenderView(worldView, worldRenderView);

		Matrix44 view = getViewTransform();

		m_primitiveRenderer->begin(m_renderView);
		m_primitiveRenderer->setClipDistance(worldRenderView.getViewFrustum().getNearZ());
		m_primitiveRenderer->pushProjection(worldRenderView.getProjection());

		// Render grid.
		const Color gridColor(0, 0, 0, 64);

		float unitsPerStep = 1.0f / 5.0f;
		int steps = int32_t(m_magnification / unitsPerStep);
		while (steps > 40)
		{
			unitsPerStep *= 2.0f;
			steps = int32_t(m_magnification / unitsPerStep);
		}

		for (int i = -1; i <= steps + 1; ++i)
		{
			float fx = float(i - steps / 2.0f) * unitsPerStep + fmod(m_cameraX, unitsPerStep);
			float fy = float(i - steps / 2.0f) * unitsPerStep + fmod(m_cameraY, unitsPerStep);
			float fz = m_magnification;

			m_primitiveRenderer->drawLine(
				Vector4(fx, -fz, 0.0f, 1.0f),
				Vector4(fx, fz, 0.0f, 1.0f),
				0.0f,
				gridColor
			);

			m_primitiveRenderer->drawLine(
				Vector4(-fz, fy, 0.0f, 1.0f),
				Vector4(fz, fy, 0.0f, 1.0f),
				0.0f,
				gridColor
			);
		}

		// @hack Translate a bit as default entity editor uses distance to calculate snap guide sizes.
		m_primitiveRenderer->pushView(translate(0.0f, 0.0f, 10.0f) * view);

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

		// Draw cameras.
		for (int i = 0; i < 4; ++i)
		{
			const Camera* camera = m_context->getCamera(i);
			if (!camera || !camera->isEnable())
				continue;

			m_primitiveRenderer->pushView(view);
			m_primitiveRenderer->pushWorld(camera->getCurrentWorld());

			m_primitiveRenderer->drawWireAabb(
				Vector4::origo(),
				Vector4(0.1f, 0.1f, 0.1f, 0.0f),
				Color(255, 255, 0, 255)
			);

			for (int j = 0; j < sizeof_array(c_cameraMeshIndices); j += 2)
			{
				int32_t i1 = c_cameraMeshIndices[j + 0] - 1;
				int32_t i2 = c_cameraMeshIndices[j + 1] - 1;

				const float* v1 = &c_cameraMeshVertices[i1 * 3];
				const float* v2 = &c_cameraMeshVertices[i2 * 3];

				m_primitiveRenderer->drawLine(
					Vector4(v1[0], v1[1], v1[2], 1.0f),
					Vector4(v2[0], v2[1], v2[2], 1.0f),
					Color(255, 255, 0, 200)
				);
			}

			m_primitiveRenderer->popWorld();
			m_primitiveRenderer->popView();
		}

		// Render entities.
		worldRenderView.setView(view);

		if (rootEntity)
		{
			m_worldRenderer->build(worldRenderView, deltaTime, rootEntity, 0);
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
