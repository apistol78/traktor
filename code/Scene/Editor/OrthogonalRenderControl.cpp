#include <limits>
#include "Core/Math/Log2.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/Settings.h"
#include "Database/Database.h"
#include "Editor/IEditor.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Scene.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/CameraMesh.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityRendererAdapter.h"
#include "Scene/Editor/IEntityEditor.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/ISceneControllerEditor.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/OrthogonalRenderControl.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"
#include "Ui/MethodHandler.h"
#include "Ui/Widget.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/KeyEvent.h"
#include "Ui/Itf/IWidget.h"
#include "World/IWorldRenderer.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const int32_t c_defaultMultiSample = 4;
const float c_cameraTranslateDeltaScale = 0.025f;
const float c_minMagnification = 0.01f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.OrthogonalRenderControl", OrthogonalRenderControl, ISceneRenderControl)

OrthogonalRenderControl::OrthogonalRenderControl()
:	m_gridEnable(true)
,	m_guideEnable(true)
,	m_mousePosition(0, 0)
,	m_mouseButton(0)
,	m_modifyCamera(false)
,	m_modifyAlternative(false)
,	m_modifyBegun(false)
,	m_multiSample(0)
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

	m_multiSample = m_context->getEditor()->getSettings()->getProperty< PropertyInteger >(L"Editor.MultiSample", c_defaultMultiSample);
	m_viewPlane = viewPlane;

	m_renderWidget = new ui::Widget();
	if (!m_renderWidget->create(parent))
		return false;

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 24;
	desc.stencilBits = 0;
	desc.multiSample = m_multiSample;
	desc.waitVBlank = false;
	desc.nativeWindowHandle = m_renderWidget->getIWidget()->getSystemHandle();

	m_renderView = m_context->getRenderSystem()->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(
		m_context->getResourceManager(),
		m_context->getRenderSystem()
	))
		return false;

	m_renderWidget->addButtonDownEventHandler(ui::createMethodHandler(this, &OrthogonalRenderControl::eventButtonDown));
	m_renderWidget->addButtonUpEventHandler(ui::createMethodHandler(this, &OrthogonalRenderControl::eventButtonUp));
	m_renderWidget->addDoubleClickEventHandler(ui::createMethodHandler(this, &OrthogonalRenderControl::eventDoubleClick));
	m_renderWidget->addMouseMoveEventHandler(ui::createMethodHandler(this, &OrthogonalRenderControl::eventMouseMove));
	m_renderWidget->addMouseWheelEventHandler(ui::createMethodHandler(this, &OrthogonalRenderControl::eventMouseWheel));
	m_renderWidget->addSizeEventHandler(ui::createMethodHandler(this, &OrthogonalRenderControl::eventSize));
	m_renderWidget->addPaintEventHandler(ui::createMethodHandler(this, &OrthogonalRenderControl::eventPaint));

	updateSettings();
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
	Ref< world::WorldEntityRenderers > worldEntityRenderers = new world::WorldEntityRenderers();
	for (RefArray< ISceneEditorProfile >::const_iterator i = m_context->getEditorProfiles().begin(); i != m_context->getEditorProfiles().end(); ++i)
	{
		RefArray< world::IEntityRenderer > entityRenderers;
		(*i)->createEntityRenderers(m_context, m_renderView, m_primitiveRenderer, entityRenderers);
		for (RefArray< world::IEntityRenderer >::iterator j = entityRenderers.begin(); j != entityRenderers.end(); ++j)
		{
			Ref< EntityRendererAdapter > entityRenderer = new EntityRendererAdapter(m_context, *j);
			worldEntityRenderers->add(entityRenderer);
		}
	}

	// Create a copy of the render settings; we don't want to enable shadows nor velocity in this view.
	world::WorldRenderSettings wrs;
	wrs = *worldRenderSettings;
	wrs.depthPassEnabled = wrs.shadowsEnabled;
	wrs.shadowsEnabled = false;

	std::wstring worldRendererTypeName = m_context->getEditor()->getSettings()->getProperty< PropertyString >(L"Editor.WorldRenderer");
	const TypeInfo* worldRendererType = TypeInfo::find(worldRendererTypeName);
	if (worldRendererType)
	{
		Ref< world::IWorldRenderer > worldRenderer = checked_type_cast< world::IWorldRenderer* >(worldRendererType->createInstance());
		T_ASSERT (worldRenderer);

		if (worldRenderer->create(
			wrs,
			worldEntityRenderers,
			m_context->getResourceManager(),
			m_context->getRenderSystem(),
			m_renderView,
			m_multiSample,
			1
		))
			m_worldRenderer = worldRenderer;
	}

	m_viewFarZ = wrs.viewFarZ;
}

void OrthogonalRenderControl::setAspect(float aspect)
{
}

bool OrthogonalRenderControl::handleCommand(const ui::Command& command)
{
	bool result = false;

	if (command == L"Editor.SettingsChanged")
		updateSettings();
	else if (command == L"Scene.Editor.EnableGrid")
		m_gridEnable = true;
	else if (command == L"Scene.Editor.DisableGrid")
		m_gridEnable = false;
	else if (command == L"Scene.Editor.EnableGuide")
		m_guideEnable = true;
	else if (command == L"Scene.Editor.DisableGuide")
		m_guideEnable = false;

	return result;
}

void OrthogonalRenderControl::update()
{
	m_renderWidget->update();
}

void OrthogonalRenderControl::updateSettings()
{
	Ref< PropertyGroup > colors = m_context->getEditor()->getSettings()->getProperty< PropertyGroup >(L"Editor.Colors");
	m_colorClear = colors->getProperty< PropertyColor >(L"Background");
	m_colorGrid = colors->getProperty< PropertyColor >(L"Grid");
	m_colorRef = colors->getProperty< PropertyColor >(L"ReferenceEdge");
	m_colorCamera = colors->getProperty< PropertyColor >(L"CameraWire");
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

Ref< EntityAdapter > OrthogonalRenderControl::pickEntity(const ui::Point& position) const
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
	else
	{
		// Handle entity picking if enabled.
		if (!m_modifyAlternative && m_context->getPickEnable())
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

void OrthogonalRenderControl::eventDoubleClick(ui::Event* event)
{
	ui::Point mousePosition = checked_type_cast< ui::MouseEvent* >(event)->getPosition();

	Ref< EntityAdapter > entityAdapter = pickEntity(mousePosition);
	if (entityAdapter && entityAdapter->isExternal())
	{
		Guid externalGuid;
		entityAdapter->getExternalGuid(externalGuid);

		Ref< db::Instance > instance = m_context->getEditor()->getSourceDatabase()->getInstance(externalGuid);
		if (instance)
			m_context->getEditor()->openEditor(instance);
	}
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
	if (m_renderView->begin(render::EtCyclop))
	{
		float tmp[4];
		m_colorClear.getRGBA32F(tmp);
		m_renderView->clear(
			render::CfColor | render::CfDepth,
			tmp,
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
		if (m_gridEnable)
		{
			float hx = worldView.width * 0.5f;
			float hy = worldView.height * 0.5f;

			Vector2 cp(m_cameraX, m_cameraY);
			Vector2 vtl(-hx, -hy), vbr(hx, hy);
			Vector2 tl = vtl - cp, br = vbr - cp;

			int32_t lx = int32_t(floorf(tl.x));
			int32_t rx = int32_t(ceilf(br.x));
			int32_t ty = int32_t(floorf(tl.y));
			int32_t by = int32_t(ceilf(br.y));

			int32_t step = nearestLog2(std::max((rx - lx) / 20, (by - ty) / 20));
			step = std::max(1, step);

			lx += abs(lx) % step;
			ty += abs(ty) % step;

			for (int32_t x = lx; x <= rx; x += step)
			{
				float fx = x + m_cameraX;

				m_primitiveRenderer->drawLine(
					Vector4(fx, vtl.y, 0.0f, 1.0f),
					Vector4(fx, vbr.y, 0.0f, 1.0f),
					(x == 0) ? 2.0f : 0.0f,
					m_colorGrid
				);
			}

			for (int32_t y = ty; y <= by; y += step)
			{
				float fy = y + m_cameraY;

				m_primitiveRenderer->drawLine(
					Vector4(vtl.x, fy, 1.0f),
					Vector4(vbr.x, fy, 1.0f),
					(y == 0) ? 2.0f : 0.0f,
					m_colorGrid
				);
			}
		}

		m_primitiveRenderer->pushView(view);

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
			}

			// Draw entity guides.
			if (m_guideEnable)
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
				m_colorCamera
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
					m_colorCamera
				);
			}

			m_primitiveRenderer->popWorld();
			m_primitiveRenderer->popView();
		}

		// Draw controller guides.
		Ref< ISceneControllerEditor > controllerEditor = m_context->getControllerEditor();
		if (controllerEditor && m_guideEnable)
			controllerEditor->draw(
				m_primitiveRenderer
			);

		// Render entities.
		worldRenderView.setTimes(scaledTime, deltaTime, 0.0f);
		worldRenderView.setView(view);

		if (rootEntity)
		{
			m_worldRenderer->build(worldRenderView, rootEntity, 0);
			m_worldRenderer->render(
				world::WrfDepthMap | world::WrfShadowMap | world::WrfVisualOpaque | world::WrfVisualAlphaBlend,
				0,
				render::EtCyclop
			);
		}

		m_primitiveRenderer->end(m_renderView);

		m_renderView->end();
		m_renderView->present();
	}

	event->consume();
}

	}
}
