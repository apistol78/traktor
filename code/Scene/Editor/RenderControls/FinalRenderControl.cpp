#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/ImageProcess/ImageProcess.h"
#include "Resource/IResourceManager.h"
#include "Scene/Scene.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/TransformChain.h"
#include "Scene/Editor/Events/FrameEvent.h"
#include "Scene/Editor/RenderControls/FinalRenderControl.h"
#include "Ui/Application.h"
#include "Ui/Command.h"
#include "Ui/Container.h"
#include "Ui/FloodLayout.h"
#include "Ui/Widget.h"
#include "Ui/AspectLayout.h"
#include "Ui/Itf/IWidget.h"
#include "World/Entity.h"
#include "World/IEntityEventManager.h"
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

const float c_defaultFieldOfView = 80.0f;
const float c_defaultMouseWheelRate = 10.0f;
const int32_t c_defaultMultiSample = 0;
const float c_minFieldOfView = 4.0f;
const float c_maxFieldOfView = 160.0f;
const float c_cameraTranslateDeltaScale = 0.025f;
const float c_cameraRotateDeltaScale = 0.01f;
const float c_deltaAdjust = 0.05f;
const float c_deltaAdjustSmall = 0.01f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.FinalRenderControl", FinalRenderControl, ISceneRenderControl)

FinalRenderControl::FinalRenderControl()
:	m_imageProcessQuality(world::QuDisabled)
,	m_shadowQuality(world::QuDisabled)
,	m_reflectionsQuality(world::QuDisabled)
,	m_motionBlurQuality(world::QuDisabled)
,	m_ambientOcclusionQuality(world::QuDisabled)
,	m_antiAliasQuality(world::QuDisabled)
,	m_fieldOfView(c_defaultFieldOfView)
,	m_mouseWheelRate(c_defaultMouseWheelRate)
,	m_multiSample(c_defaultMultiSample)
,	m_invertPanY(false)
,	m_dirtySize(0, 0)
{
}

bool FinalRenderControl::create(ui::Widget* parent, SceneEditorContext* context, int32_t cameraId)
{
	m_context = context;
	T_ASSERT(m_context);

	const PropertyGroup* settings = m_context->getEditor()->getSettings();
	T_ASSERT(settings);

	m_fieldOfView = std::max< float >(settings->getProperty< float >(L"SceneEditor.FieldOfView", c_defaultFieldOfView), c_minFieldOfView);
	m_mouseWheelRate = settings->getProperty< float >(L"SceneEditor.MouseWheelRate", c_defaultMouseWheelRate);
	m_multiSample = settings->getProperty< int32_t >(L"Editor.MultiSample", c_defaultMultiSample);

	m_containerAspect = new ui::Container();
	m_containerAspect->create(parent, ui::WsNone, new ui::FloodLayout());

	m_renderWidget = new ui::Widget();
	if (!m_renderWidget->create(m_containerAspect, ui::WsWantAllInput))
		return false;

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 24;
	desc.stencilBits = 0;
	desc.multiSample = m_multiSample;
	desc.waitVBlanks = 0;
	desc.syswin = m_renderWidget->getIWidget()->getSystemWindow();

	m_renderView = m_context->getRenderSystem()->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_renderWidget->addEventHandler< ui::MouseButtonDownEvent >(this, &FinalRenderControl::eventButtonDown);
	m_renderWidget->addEventHandler< ui::MouseButtonUpEvent >(this, &FinalRenderControl::eventButtonUp);
	m_renderWidget->addEventHandler< ui::MouseDoubleClickEvent >(this, &FinalRenderControl::eventDoubleClick);
	m_renderWidget->addEventHandler< ui::MouseMoveEvent >(this, &FinalRenderControl::eventMouseMove);
	m_renderWidget->addEventHandler< ui::MouseWheelEvent >(this, &FinalRenderControl::eventMouseWheel);
	m_renderWidget->addEventHandler< ui::SizeEvent >(this, &FinalRenderControl::eventSize);
	m_renderWidget->addEventHandler< ui::PaintEvent >(this, &FinalRenderControl::eventPaint);

	if (m_context->getDocument()->getInstance(0)->getPrimaryType() == &type_of< SceneAsset >())
	{
		resource::Id< scene::Scene > sceneId(m_context->getDocument()->getInstance(0)->getGuid());
		if (!m_context->getResourceManager()->bind(
			sceneId,
			m_sceneInstance
		))
			return false;
	}

	updateSettings();
	updateWorldRenderer();

	m_worldRenderView.setIndex(cameraId);

	m_camera = m_context->getCamera(cameraId);
	m_camera->setEnable(true);
	m_timer.start();

	return true;
}

void FinalRenderControl::destroy()
{
	if (m_camera)
	{
		m_camera->setEnable(false);
		m_camera = nullptr;
	}

	safeDestroy(m_worldRenderer);
	safeClose(m_renderView);
	safeDestroy(m_containerAspect);
}

void FinalRenderControl::updateWorldRenderer()
{
	safeDestroy(m_worldRenderer);

	if (!m_sceneInstance)
		return;

	ui::Size sz = m_renderWidget->getInnerRect().getSize();
	if (sz.cx <= 0 || sz.cy <= 0)
		return;

	m_worldRenderSettings = *(m_sceneInstance->getWorldRenderSettings());

	// Create entity renderers.
	Ref< world::WorldEntityRenderers > worldEntityRenderers = new world::WorldEntityRenderers();
	for (auto editorProfile : m_context->getEditorProfiles())
	{
		RefArray< world::IEntityRenderer > entityRenderers;
		editorProfile->createEntityRenderers(m_context, m_renderView, nullptr, entityRenderers);
		for (auto entityRenderer : entityRenderers)
			worldEntityRenderers->add(entityRenderer);
	}

	const PropertyGroup* settings = m_context->getEditor()->getSettings();
	T_ASSERT(settings);

	std::wstring worldRendererTypeName = settings->getProperty< std::wstring >(L"SceneEditor.WorldRendererType", L"traktor.world.WorldRendererDeferred");

	const TypeInfo* worldRendererType = TypeInfo::find(worldRendererTypeName.c_str());
	if (!worldRendererType)
		return;

	Ref< world::IWorldRenderer > worldRenderer = dynamic_type_cast< world::IWorldRenderer* >(worldRendererType->createInstance());
	if (!worldRenderer)
		return;

	world::WorldCreateDesc wcd;
	wcd.worldRenderSettings = &m_worldRenderSettings;
	wcd.entityRenderers = worldEntityRenderers;
	wcd.motionBlurQuality = m_motionBlurQuality;
	wcd.shadowsQuality = m_shadowQuality;
	wcd.reflectionsQuality = m_reflectionsQuality;
	wcd.ambientOcclusionQuality = m_ambientOcclusionQuality;
	wcd.antiAliasQuality = m_antiAliasQuality;
	wcd.imageProcessQuality = m_imageProcessQuality;
	wcd.width = sz.cx;
	wcd.height = sz.cy;
	wcd.multiSample = m_multiSample;
	wcd.frameCount = 1;
	wcd.allTargetsPersistent = true;

	if (worldRenderer->create(
		m_context->getResourceManager(),
		m_context->getRenderSystem(),
		wcd
	))
	{
		m_worldRenderer = worldRenderer;
	}
}

void FinalRenderControl::setWorldRendererType(const TypeInfo& worldRendererType)
{
}

void FinalRenderControl::setAspect(float aspect)
{
	if (aspect > 0.0f)
		m_containerAspect->setLayout(new ui::AspectLayout(aspect));
	else
		m_containerAspect->setLayout(new ui::FloodLayout());

	m_containerAspect->update();
}

void FinalRenderControl::setQuality(world::Quality imageProcessQuality, world::Quality shadowQuality, world::Quality reflectionsQuality, world::Quality motionBlurQuality, world::Quality ambientOcclusionQuality, world::Quality antiAliasQuality)
{
	m_imageProcessQuality = imageProcessQuality;
	m_shadowQuality = shadowQuality;
	m_reflectionsQuality = reflectionsQuality;
	m_motionBlurQuality = motionBlurQuality;
	m_ambientOcclusionQuality = ambientOcclusionQuality;
	m_antiAliasQuality = antiAliasQuality;
	updateWorldRenderer();
}

bool FinalRenderControl::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.SettingsChanged")
		updateSettings();
	return false;
}

void FinalRenderControl::update()
{
	m_renderWidget->update(nullptr, false);
}

bool FinalRenderControl::calculateRay(const ui::Point& position, Vector4& outWorldRayOrigin, Vector4& outWorldRayDirection) const
{
	return false;
}

bool FinalRenderControl::calculateFrustum(const ui::Rect& rc, Frustum& outWorldFrustum) const
{
	return false;
}

bool FinalRenderControl::hitTest(const ui::Point& position) const
{
	return m_renderWidget->hitTest(position);
}

void FinalRenderControl::moveCamera(MoveCameraMode mode, const Vector4& mouseDelta, const Vector4& viewDelta)
{
	Vector4 delta = mouseDelta;
	switch (mode)
	{
	case McmRotate:
		delta *= Scalar(c_cameraRotateDeltaScale);
		m_camera->rotate(delta.y(), delta.x());
		break;

	case McmMoveXZ:
		delta *= Scalar(c_cameraTranslateDeltaScale);
		m_camera->move(delta.shuffle< 0, 2, 1, 3 >());
		break;

	case McmMoveXY:
		if (!m_invertPanY)
			delta *= Vector4(c_cameraTranslateDeltaScale, -c_cameraTranslateDeltaScale, 0.0f, 0.0f);
		else
			delta *= Vector4(c_cameraTranslateDeltaScale, c_cameraTranslateDeltaScale, 0.0f, 0.0f);
		m_camera->move(delta.shuffle< 0, 1, 2, 3 >());
		break;
	}
}

void FinalRenderControl::showSelectionRectangle(const ui::Rect& rect)
{
}

void FinalRenderControl::getDebugTargets(std::vector< render::DebugTarget >& outDebugTargets)
{
}

void FinalRenderControl::setDebugTarget(const render::DebugTarget* debugTarget)
{
}

void FinalRenderControl::updateSettings()
{
	const PropertyGroup* settings = m_context->getEditor()->getSettings();
	T_ASSERT(settings);

	m_colorClear = settings->getProperty< Color4ub >(L"Editor.Colors/Background");
	m_invertPanY = settings->getProperty< bool >(L"SceneEditor.InvertPanY");
	m_fieldOfView = std::max< float >(settings->getProperty< float >(L"SceneEditor.FieldOfView", c_defaultFieldOfView), c_minFieldOfView);
	m_mouseWheelRate = settings->getProperty< float >(L"SceneEditor.MouseWheelRate", c_defaultMouseWheelRate);

	updateWorldRenderer();
}

Matrix44 FinalRenderControl::getProjectionTransform() const
{
	return m_worldRenderView.getProjection();
}

Matrix44 FinalRenderControl::getViewTransform() const
{
	return m_camera->getView().toMatrix44();
}

void FinalRenderControl::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventButtonDown(this, m_renderWidget, event, m_context, transformChain);
}

void FinalRenderControl::eventButtonUp(ui::MouseButtonUpEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventButtonUp(this, m_renderWidget, event, m_context, transformChain);
}

void FinalRenderControl::eventDoubleClick(ui::MouseDoubleClickEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventDoubleClick(this, m_renderWidget, event, m_context, transformChain);
}

void FinalRenderControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	TransformChain transformChain;
	transformChain.pushProjection(getProjectionTransform());
	transformChain.pushView(getViewTransform());
	m_model.eventMouseMove(this, m_renderWidget, event, m_context, transformChain);
}

void FinalRenderControl::eventMouseWheel(ui::MouseWheelEvent* event)
{
	int32_t rotation = event->getRotation();

	if (m_context->getEditor()->getSettings()->getProperty(L"SceneEditor.InvertMouseWheel"))
		rotation = -rotation;

	m_camera->move(Vector4(0.0f, 0.0f, rotation * -m_mouseWheelRate, 0.0f));
	m_context->raiseCameraMoved();
	m_context->raiseRedraw();
}

void FinalRenderControl::eventSize(ui::SizeEvent* event)
{
	if (!m_renderView || !m_renderWidget->isVisible(true))
		return;

	ui::Size sz = event->getSize();

	// Don't update world renderer if, in fact, size hasn't changed.
	if (sz.cx == m_dirtySize.cx && sz.cy == m_dirtySize.cy)
		return;

	m_renderView->reset(sz.cx, sz.cy);
	m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));

	safeDestroy(m_worldRenderer);

	m_dirtySize = sz;
}

void FinalRenderControl::eventPaint(ui::PaintEvent* event)
{
	float colorClear[4];
	float deltaTime = float(m_timer.getDeltaTime());
	float scaledTime = m_context->getTime();
	float scaledDeltaTime = m_context->isPlaying() ? deltaTime * m_context->getTimeScale() : 0.0f;

	m_colorClear.getRGBA32F(colorClear);

	if (!m_sceneInstance || !m_renderView)
		return;

	// Reload scene if changed.
	if (m_sceneInstance.changed())
	{
		m_worldRenderer = nullptr;
		m_sceneInstance.consume();
	}

	const world::WorldRenderSettings* worldRenderSettings = m_sceneInstance->getWorldRenderSettings();
	ui::Size sz = m_renderWidget->getInnerRect().getSize();

	// Lazy create world renderer.
	if (!m_worldRenderer)
	{
		updateWorldRenderer();
		if (!m_worldRenderer)
			return;
	}

	// Update scene entities; final render control has it's own set of entities thus
	// need to manually update those.
	world::UpdateParams update;
	update.totalTime = scaledTime;
	update.deltaTime = deltaTime;
	update.alternateTime = scaledTime;

	m_sceneInstance->updateController(update);
	m_sceneInstance->updateEntity(update);

	// Update world render view.
	m_worldRenderView.setPerspective(
		float(sz.cx),
		float(sz.cy),
		float(sz.cx) / sz.cy,
		deg2rad(m_fieldOfView),
		worldRenderSettings->viewNearZ,
		worldRenderSettings->viewFarZ
	);

	// Get current transformations.
	Matrix44 projection = getProjectionTransform();
	Matrix44 view = getViewTransform();

	// Build world.
	m_worldRenderView.setTimes(scaledTime, deltaTime, 1.0f);
	m_worldRenderView.setView(m_worldRenderView.getView(), view);

	// Build frame from scene entities.
	m_worldRenderer->attach(m_sceneInstance->getRootEntity());
	m_context->getEntityEventManager()->attach(m_worldRenderer);
	m_worldRenderer->build(m_worldRenderView, 0);

	// Render world.
	if (m_renderView->begin(nullptr))
	{
		// Set post process parameters from scene instance.
		render::ImageProcess* postProcess = m_worldRenderer->getVisualImageProcess();
		if (postProcess)
		{
			for (const auto& imageProcessParam : m_sceneInstance->getImageProcessParams())
				postProcess->setTextureParameter(imageProcessParam.first, imageProcessParam.second);
		}

		m_worldRenderer->beginRender(m_renderView, 0, Color4f(colorClear[0], colorClear[1], colorClear[2], colorClear[3]));
		m_worldRenderer->render(m_renderView, 0);
		m_worldRenderer->endRender(m_renderView, 0, deltaTime);

		m_renderView->end();
		m_renderView->present();
	}

	event->consume();
}

	}
}
