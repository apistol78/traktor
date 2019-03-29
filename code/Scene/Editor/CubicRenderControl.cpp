#include <cstring>
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Format.h"
#include "Core/Misc/EnterLeave.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/GammaFilter.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Drawing/Filters/TransformFilter.h"
#include "Editor/IEditor.h"
#include "Editor/TypeBrowseFilter.h"
#include "Render/ICubeTexture.h"
#include "Render/IndexBuffer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Render/RenderTargetSet.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Editor/Texture/IrradianceProbeAsset.h"
#include "Render/Editor/Texture/RadianceProbeAsset.h"
#include "Render/ImageProcess/ImageProcess.h"
#include "Resource/IResourceManager.h"
#include "Scene/Scene.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityRendererAdapter.h"
#include "Scene/Editor/EntityRendererCache.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/ISceneControllerEditor.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/IEntityEditor.h"
#include "Scene/Editor/CubicRenderControl.h"
#include "Scene/Editor/ProbeLayerAttribute.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/TransformChain.h"
#include "Scene/Editor/Events/FrameEvent.h"
#include "Ui/Command.h"
#include "Ui/Container.h"
#include "Ui/FileDialog.h"
#include "Ui/TableLayout.h"
#include "Ui/Widget.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "World/Entity.h"
#include "World/IEntityEventManager.h"
#include "World/IWorldRenderer.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldRenderView.h"
#include "World/Entity/CameraComponent.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const int32_t c_previewFaceSize = 256;
const int32_t c_saveFaceSize = 256;

const resource::Id< render::Shader > c_idProbeTexturePreview(Guid(L"{2F69EAE9-FA20-3244-9B8C-C803E538C19F}"));

#pragma pack(1)
struct Vertex
{
	float xyz[3];
};
#pragma pack()

const Vertex c_cubeVertices[] =
{
	{ -1, -1, 1 },
	{ -1, -1, -1 },
	{ -1, 1, -1 },
	{ -1, 1, 1 },
	{ 1, -1, 1 },
	{ 1, -1, -1 },
	{ 1, 1, -1 },
	{ 1, 1, 1 },
};

const uint16_t c_cubeIndices[] =
{
	3, 1, 0,
	3, 2, 1,
	1, 4, 0,
	1, 5, 4,
	2, 5, 1,
	2, 6, 5,
	2, 3, 7,
	6, 2, 7,
	3, 0, 4,
	7, 3, 4,
	5, 7, 4,
	6, 7, 5,
};

int32_t translateMouseButton(int32_t uimb)
{
	if (uimb == ui::MbtLeft)
		return 1;
	else if (uimb == ui::MbtRight)
		return 2;
	else if (uimb == (ui::MbtLeft | ui::MbtRight) || uimb == ui::MbtMiddle)
		return 3;
	else
		return 0;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CubicRenderControl", CubicRenderControl, ISceneRenderControl)

CubicRenderControl::CubicRenderControl()
:	m_imageProcessQuality(world::QuDisabled)
,	m_shadowQuality(world::QuDisabled)
,	m_reflectionsQuality(world::QuDisabled)
,	m_motionBlurQuality(world::QuDisabled)
,	m_ambientOcclusionQuality(world::QuDisabled)
,	m_antiAliasQuality(world::QuDisabled)
,	m_mouseButton(0)
,	m_previewHead(0.0f)
,	m_previewPitch(0.0f)
,	m_dirtySize(0, 0)
{
}

bool CubicRenderControl::create(ui::Widget* parent, SceneEditorContext* context)
{
	m_context = context;
	T_ASSERT(m_context);

	const PropertyGroup* settings = m_context->getEditor()->getSettings();
	T_ASSERT(settings);

	m_container = new ui::Container();
	if (!m_container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	m_toolBar = new ui::ToolBar();
	if (!m_toolBar->create(m_container))
		return false;

	m_toolBar->addItem(new ui::ToolBarButton(L"Capture at origo", ui::Command(L"Scene.Editor.CaptureAtOrigo"), ui::ToolBarButton::BsText));
	m_toolBar->addItem(new ui::ToolBarButton(L"Capture at selected", ui::Command(L"Scene.Editor.CaptureAtSelected"), ui::ToolBarButton::BsText));
	m_toolBar->addItem(new ui::ToolBarButton(L"Save cubemap...", ui::Command(L"Scene.Editor.SaveCubeMap"), ui::ToolBarButton::BsText));
	m_toolBar->addItem(new ui::ToolBarButton(L"Update probe...", ui::Command(L"Scene.Editor.UpdateProbe"), ui::ToolBarButton::BsText));
	m_toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &CubicRenderControl::eventToolClick);

	m_renderWidget = new ui::Widget();
	if (!m_renderWidget->create(m_container))
		return false;

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 24;
	desc.stencilBits = 0;
	desc.multiSample = 0;
	desc.waitVBlanks = 0;
	desc.syswin = m_renderWidget->getIWidget()->getSystemWindow();

	m_renderView = m_context->getRenderSystem()->createRenderView(desc);
	if (!m_renderView)
		return false;

	render::RenderTargetSetCreateDesc rtscd;
	rtscd.count = 6;
	rtscd.width = c_previewFaceSize;
	rtscd.height = c_previewFaceSize;
	rtscd.multiSample = 0;
	rtscd.createDepthStencil = true;
	rtscd.usingDepthStencilAsTexture = false;
	rtscd.usingPrimaryDepthStencil = false;
	rtscd.preferTiled = false;
	rtscd.ignoreStencil = true;
	rtscd.generateMips = false;
	rtscd.targets[0].format = render::TfR32G32B32A32F;
	rtscd.targets[0].sRGB = false;
	rtscd.targets[1].format = render::TfR32G32B32A32F;
	rtscd.targets[1].sRGB = false;
	rtscd.targets[2].format = render::TfR32G32B32A32F;
	rtscd.targets[2].sRGB = false;
	rtscd.targets[3].format = render::TfR32G32B32A32F;
	rtscd.targets[3].sRGB = false;
	rtscd.targets[4].format = render::TfR32G32B32A32F;
	rtscd.targets[4].sRGB = false;
	rtscd.targets[5].format = render::TfR32G32B32A32F;
	rtscd.targets[5].sRGB = false;

	m_renderTargetSet = m_context->getRenderSystem()->createRenderTargetSet(rtscd);
	if (!m_renderTargetSet)
		return false;

	render::CubeTextureCreateDesc ctcd;
	ctcd.side = c_previewFaceSize;
	ctcd.mipCount = 1;
	ctcd.format = render::TfR32G32B32A32F;
	ctcd.sRGB = false;
	ctcd.immutable = false;

	m_cubeMapTexture = m_context->getRenderSystem()->createCubeTexture(ctcd);
	if (!m_cubeMapTexture)
		return false;

	for (int32_t i = 0; i < 6; ++i)
	{
		m_cubeImages[i] = new drawing::Image(drawing::PixelFormat::getRGBAF32(), c_previewFaceSize, c_previewFaceSize);
		m_cubeImages[i]->clear(Color4f(0.0f, 0.0f, 0.0f, 1.0f));
	}

	AlignedVector< render::VertexElement > vertexDecl(1);
	vertexDecl[0] = render::VertexElement(render::DuPosition, render::DtFloat3, 0);

	m_vertexBuffer = m_context->getRenderSystem()->createVertexBuffer(vertexDecl, sizeof(c_cubeVertices), false);
	std::memcpy(m_vertexBuffer->lock(), c_cubeVertices, sizeof(c_cubeVertices));
	m_vertexBuffer->unlock();

	m_indexBuffer = m_context->getRenderSystem()->createIndexBuffer(render::ItUInt16, sizeof(c_cubeIndices), false);
	std::memcpy(m_indexBuffer->lock(), c_cubeIndices, sizeof(c_cubeIndices));
	m_indexBuffer->unlock();

	if (!m_context->getResourceManager()->bind(c_idProbeTexturePreview, m_shader))
		return false;

	m_renderWidget->addEventHandler< ui::MouseButtonDownEvent >(this, &CubicRenderControl::eventButtonDown);
	m_renderWidget->addEventHandler< ui::MouseButtonUpEvent >(this, &CubicRenderControl::eventButtonUp);
	m_renderWidget->addEventHandler< ui::MouseMoveEvent >(this, &CubicRenderControl::eventMouseMove);
	m_renderWidget->addEventHandler< ui::SizeEvent >(this, &CubicRenderControl::eventSize);
	m_renderWidget->addEventHandler< ui::PaintEvent >(this, &CubicRenderControl::eventPaint);

	updateWorldRenderer();

	m_timer.start();
	return true;
}

void CubicRenderControl::destroy()
{
	safeDestroy(m_worldRenderer);
	safeClose(m_renderView);
	safeDestroy(m_renderWidget);
}

void CubicRenderControl::updateWorldRenderer()
{
	safeDestroy(m_worldRenderer);

	Ref< scene::Scene > sceneInstance = m_context->getScene();
	if (!sceneInstance)
		return;

	m_worldRenderSettings = *sceneInstance->getWorldRenderSettings();

	// Remove all post processing.
	for (int32_t i = 0; i < sizeof_array(m_worldRenderSettings.imageProcess); ++i)
		m_worldRenderSettings.imageProcess[i] = resource::Id< render::ImageProcessSettings >();

	// Create entity renderers.
	Ref< EntityRendererCache > entityRendererCache = new EntityRendererCache(m_context);
	Ref< world::WorldEntityRenderers > worldEntityRenderers = new world::WorldEntityRenderers();
	for (auto profile : m_context->getEditorProfiles())
	{
		RefArray< world::IEntityRenderer > entityRenderers;
		profile->createEntityRenderers(m_context, m_renderView, nullptr, entityRenderers);
		for (auto entityRenderer : entityRenderers)
		{
			Ref< EntityRendererAdapter > entityRendererAdapter = new EntityRendererAdapter(entityRendererCache, entityRenderer, [&](const EntityAdapter* adapter) {
				return adapter->isVisible() && adapter->getLayerAttribute< ProbeLayerAttribute >() != nullptr;
			});
			worldEntityRenderers->add(entityRendererAdapter);
		}
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
	wcd.toneMapQuality = world::QuDisabled;
	wcd.motionBlurQuality = world::QuDisabled;
	wcd.reflectionsQuality = world::QuDisabled;
	wcd.shadowsQuality = world::QuDisabled;
	wcd.ambientOcclusionQuality = world::QuDisabled;
	wcd.antiAliasQuality = world::QuDisabled;
	wcd.imageProcessQuality = world::QuDisabled;
	wcd.width = m_renderTargetSet->getWidth();
	wcd.height = m_renderTargetSet->getHeight();
	wcd.multiSample = 0;
	wcd.frameCount = 1;
	wcd.allTargetsPersistent = false;
	wcd.sharedDepthStencil = m_renderTargetSet;

	if (worldRenderer->create(
		m_context->getResourceManager(),
		m_context->getRenderSystem(),
		m_renderView,
		wcd
	))
	{
		m_worldRenderer = worldRenderer;
	}
}

void CubicRenderControl::setAspect(float aspect)
{
}

void CubicRenderControl::setQuality(world::Quality imageProcessQuality, world::Quality shadowQuality, world::Quality reflectionsQuality, world::Quality motionBlurQuality, world::Quality ambientOcclusionQuality, world::Quality antiAliasQuality)
{
	m_imageProcessQuality = imageProcessQuality;
	m_shadowQuality = shadowQuality;
	m_reflectionsQuality = reflectionsQuality;
	m_motionBlurQuality = motionBlurQuality;
	m_ambientOcclusionQuality = ambientOcclusionQuality;
	m_antiAliasQuality = antiAliasQuality;
	updateWorldRenderer();
}

bool CubicRenderControl::handleCommand(const ui::Command& command)
{
	if (command == L"Scene.Editor.CaptureAtOrigo")
	{
		capture(Vector4::origo());
		capture(Vector4::origo());
		update();
	}
	else if (command == L"Scene.Editor.CaptureAtSelected")
	{
		RefArray< EntityAdapter > selectedEntities;
		if (m_context->getEntities(selectedEntities, SceneEditorContext::GfDescendants | SceneEditorContext::GfSelectedOnly) > 0)
		{
			Vector4 pivot = selectedEntities.front()->getTransform().inverse().translation().xyz1();
			capture(pivot);
			capture(pivot);
			update();
		}
		else
			log::warning << L"No entity selected; cannot capture." << Endl;
	}
	else if (command == L"Scene.Editor.SaveCubeMap")
	{
		Path fileName;

		ui::FileDialog saveAsDialog;
		saveAsDialog.create(
			m_container,
			type_name(this),
			L"Save cube map as...",
			L"All files (*.*);*.*",
			true
		);
		if (saveAsDialog.showModal(fileName) != ui::DrOk)
		{
			saveAsDialog.destroy();
			return true;
		}
		saveAsDialog.destroy();

		Ref< drawing::Image > cm = new drawing::Image(drawing::PixelFormat::getRGBAF32(), 6 * c_saveFaceSize, c_saveFaceSize);
		for (int32_t i = 0; i < 6; ++i)
		{
			Ref< drawing::Image > faceImage = m_cubeImages[i]->clone();

			drawing::ScaleFilter scaleFilter(c_saveFaceSize, c_saveFaceSize, drawing::ScaleFilter::MnAverage, drawing::ScaleFilter::MgLinear);
			faceImage->apply(&scaleFilter);

			drawing::TransformFilter transformFilter(Color4f(2.0f, 2.0f, 2.0f, 1.0f), Color4f(0.0f, 0.0f, 0.0f, 0.0f));
			faceImage->apply(&transformFilter);

			drawing::GammaFilter gammaFilter(2.2f);
			faceImage->apply(&gammaFilter);

			cm->copy(faceImage, i * c_saveFaceSize, 0, 0, 0, c_saveFaceSize, c_saveFaceSize);
		}

		if (cm->save(fileName))
			log::info << L"Cube map saved successfully as \"" << fileName.getPathName() << L"\"." << Endl;
		else
			log::error << L"Unable to save cube map \"" << fileName.getPathName() << L"\"." << Endl;
	}
	else if (command == L"Scene.Editor.UpdateProbe")
	{
		editor::TypeBrowseFilter filter(makeTypeInfoSet<
			render::IrradianceProbeAsset,
			render::RadianceProbeAsset
		>());
		Ref< db::Instance > probeAssetInstance = m_context->getEditor()->browseInstance(&filter);
		if (!probeAssetInstance)
			return true;

		if (!probeAssetInstance->checkout())
			return true;

		Ref< IStream > dataStream = probeAssetInstance->writeData(L"Data");
		if (!dataStream)
		{
			probeAssetInstance->revert();
			return true;
		}

		Ref< drawing::Image > cm = new drawing::Image(drawing::PixelFormat::getRGBAF32(), 6 * c_saveFaceSize, c_saveFaceSize);
		for (int32_t i = 0; i < 6; ++i)
		{
			Ref< drawing::Image > faceImage = m_cubeImages[i]->clone();

			drawing::ScaleFilter scaleFilter(c_saveFaceSize, c_saveFaceSize, drawing::ScaleFilter::MnAverage, drawing::ScaleFilter::MgLinear);
			faceImage->apply(&scaleFilter);

			drawing::TransformFilter transformFilter(Color4f(2.0f, 2.0f, 2.0f, 1.0f), Color4f(0.0f, 0.0f, 0.0f, 0.0f));
			faceImage->apply(&transformFilter);

			drawing::GammaFilter gammaFilter(2.2f);
			faceImage->apply(&gammaFilter);

			cm->copy(faceImage, i * c_saveFaceSize, 0, 0, 0, c_saveFaceSize, c_saveFaceSize);
		}
		if (!cm->save(dataStream, L"tri"))
		{
			probeAssetInstance->revert();
			return true;
		}

		probeAssetInstance->commit();

		m_context->getEditor()->buildAsset(probeAssetInstance->getGuid(), false);

		log::info << L"Probe \"" << probeAssetInstance->getGuid().format() << L"\" updated successfully." << Endl;
	}
	else
		return false;

	return true;
}

void CubicRenderControl::update()
{
	m_renderWidget->update(nullptr, false);
}

bool CubicRenderControl::calculateRay(const ui::Point& position, Vector4& outWorldRayOrigin, Vector4& outWorldRayDirection) const
{
	return false;
}

bool CubicRenderControl::calculateFrustum(const ui::Rect& rc, Frustum& outWorldFrustum) const
{
	return false;
}

bool CubicRenderControl::hitTest(const ui::Point& position) const
{
	return m_renderWidget->hitTest(position);
}

void CubicRenderControl::moveCamera(MoveCameraMode mode, const Vector4& mouseDelta, const Vector4& viewDelta)
{
}

void CubicRenderControl::showSelectionRectangle(const ui::Rect& rect)
{
}

void CubicRenderControl::capture(const Vector4& pivot)
{
	Ref< scene::Scene > sceneInstance = m_context->getScene();
	if (!sceneInstance || !m_renderView)
		return;

	// Lazy create world renderer.
	if (!m_worldRenderer)
	{
		updateWorldRenderer();
		if (!m_worldRenderer)
			return;
	}

	// Create world render view.
	const world::WorldRenderSettings* worldRenderSettings = sceneInstance->getWorldRenderSettings();
	m_worldRenderView.setPerspective(
		c_previewFaceSize,
		c_previewFaceSize,
		1.0f,
		deg2rad(90.0f),
		worldRenderSettings->viewNearZ,
		worldRenderSettings->viewFarZ
	);

	// Render world.
	if (m_renderView->begin())
	{
		// Render all faces of cube map.
		for (int32_t face = 0; face < 6; ++face)
		{
			if (m_renderView->begin(m_renderTargetSet, face))
			{
				const Color4f clearColor(0.0f, 0.0f, 0.0f, 1.0f);
				m_renderView->clear(render::CfColor | render::CfDepth, &clearColor, 1.0f, 0);

				Matrix44 view;
				switch (face)
				{
				case 0:	// +X
					view = rotateY(deg2rad(-90.0f));
					break;
				case 1:	// -X
					view = rotateY(deg2rad( 90.0f));
					break;
				case 2:	// +Y
					view = rotateX(deg2rad( 90.0f));
					break;
				case 3: // -Y
					view = rotateX(deg2rad(-90.0f));
					break;
				case 4:	// +Z
					view = Matrix44::identity();
					break;
				case 5:	// -Z
					view = rotateY(deg2rad(180.0f));
					break;
				}

				// Move to pivot point.
				view = view * translate(pivot);

				// Render entities.
				m_worldRenderView.setTimes(0.0f, 1.0f / 60.0f, 0.0f);
				m_worldRenderView.setView(view, view);

				Ref< scene::Scene > sceneInstance = m_context->getScene();
				if (sceneInstance)
				{
					// Build frame from scene entities.
					m_worldRenderer->beginBuild();
					m_worldRenderer->build(sceneInstance->getRootEntity());
					m_context->getEntityEventManager()->build(m_worldRenderer);
					m_worldRenderer->endBuild(m_worldRenderView, 0);

					// Set post process parameters from scene instance.
					render::ImageProcess* postProcess = m_worldRenderer->getVisualImageProcess();
					if (postProcess)
					{
						for (const auto param : sceneInstance->getImageProcessParams())
							postProcess->setTextureParameter(param.first, param.second);
					}
				}

				m_worldRenderer->beginRender(0, clearColor);
				m_worldRenderer->render(0);
				m_worldRenderer->endRender(0, 1.0f / 60.0f);

				m_renderView->end();
			}
		}

		// Download each target and update cube texture.
		for (int32_t side = 0; side < 6; ++side)
		{
			if (!m_renderTargetSet->read(side, m_cubeImages[side]->getData()))
				log::error << L"Unable to read render target " << side << L" into cube image." << Endl;

			m_cubeImages[side]->clearAlpha(1.0f);

			render::ITexture::Lock lock;
			if (m_cubeMapTexture->lock(side, 0, lock))
			{
				std::memcpy(lock.bits, m_cubeImages[side]->getData(), m_cubeImages[side]->getDataSize());
				m_cubeMapTexture->unlock(side, 0);
			}
			else
				log::error << L"Unable to lock cubemap side " << side << L"." << Endl;
		}

		m_renderView->end();
		m_renderView->present();
	}
}

void CubicRenderControl::eventToolClick(ui::ToolBarButtonClickEvent* event)
{
	handleCommand(event->getCommand());
}

void CubicRenderControl::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	m_mousePosition0 =
	m_mousePosition = event->getPosition();
	m_mouseButton = translateMouseButton(event->getButton());

	m_renderWidget->setCapture();
	m_renderWidget->setFocus();
	m_renderWidget->update();
}

void CubicRenderControl::eventButtonUp(ui::MouseButtonUpEvent* event)
{
	m_mousePosition0 =
	m_mousePosition = ui::Point(0, 0);
	m_mouseButton = 0;

	if (m_renderWidget->hasCapture())
		m_renderWidget->releaseCapture();

	m_renderWidget->update();
}

void CubicRenderControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	if (!m_renderWidget->hasCapture())
		return;

	ui::Point mousePosition = event->getPosition();

	Vector4 mouseDelta(
		float(m_mousePosition.x - mousePosition.x),
		float(m_mousePosition.y - mousePosition.y),
		0.0f,
		0.0f
	);

	ui::Rect innerRect = m_renderWidget->getInnerRect();
	Vector2 screenPosition(2.0f * float(mousePosition.x) / innerRect.getWidth() - 1.0f, 1.0f - 2.0f * float(mousePosition.y) / innerRect.getHeight());
	Vector4 clipDelta = mouseDelta * Vector4(5.0f / innerRect.getWidth(), 5.0f / innerRect.getHeight(), 0.0f, 0.0f);

	m_previewHead += clipDelta.x();
	m_previewPitch += clipDelta.y();

	m_mousePosition = mousePosition;
}

void CubicRenderControl::eventSize(ui::SizeEvent* event)
{
	if (!m_renderView || !m_renderWidget->isVisible(true))
		return;

	ui::Size sz = event->getSize();

	// Don't update world renderer if, in fact, size hasn't changed.
	if (sz.cx == m_dirtySize.cx && sz.cy == m_dirtySize.cy)
		return;

	m_renderView->reset(sz.cx, sz.cy);
	m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));

	m_dirtySize = sz;
}

void CubicRenderControl::eventPaint(ui::PaintEvent* event)
{
	if (!m_renderView)
		return;

	//capture(Vector4::origo());

	// Render world.
	if (m_renderView->begin())
	{
		// Render cube preview.
		const Color4f clearColor(0.3f, 0.3f, 0.3f, 1.0f);
		m_renderView->clear(render::CfColor | render::CfDepth, &clearColor, 1.0f, 0);

		ui::Size sz = m_renderWidget->getInnerRect().getSize();
		float aspect = float(sz.cx) / sz.cy;

		m_shader->setMatrixParameter(L"World", translate(0.0f, 0.0f, 3.5f) * rotateX(m_previewPitch) * rotateY(m_previewHead));
		m_shader->setMatrixParameter(L"Projection", perspectiveLh(deg2rad(80.0f), aspect, 0.1f, 10.0f));
		m_shader->setTextureParameter(L"Texture", m_cubeMapTexture);
		m_shader->draw(m_renderView, m_vertexBuffer, m_indexBuffer, render::Primitives(render::PtTriangles, 0, 12, 0, 7));

		m_renderView->end();
		m_renderView->present();
	}

	event->consume();
}

	}
}
