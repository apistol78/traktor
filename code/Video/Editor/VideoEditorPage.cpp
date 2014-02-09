#include "Core/Io/FileSystem.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/ISimpleTexture.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Resource/ShaderFactory.h"
#include "Resource/ResourceManager.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/MethodHandler.h"
#include "Ui/Widget.h"
#include "Ui/Events/IdleEvent.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/Itf/IWidget.h"
#include "Video/Video.h"
#include "Video/Decoders/VideoDecoderTheora.h"
#include "Video/Editor/VideoAsset.h"
#include "Video/Editor/VideoEditorPage.h"

namespace traktor
{
	namespace video
	{
		namespace
		{

const resource::Id< render::Shader > c_idShaderMovie(Guid(L"{71682019-EB26-234C-8B48-0638F50DA662}"));

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.video.VideoEditorPage", VideoEditorPage, editor::IEditorPage)

VideoEditorPage::VideoEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
{
}

bool VideoEditorPage::create(ui::Container* parent)
{
	Ref< VideoAsset > asset = m_document->getObject< VideoAsset >(0);
	T_ASSERT (asset);

	render::IRenderSystem* renderSystem = m_editor->getStoreObject< render::IRenderSystem >(L"RenderSystem");
	if (!renderSystem)
		return false;

	// Open file stream to source video.
	std::wstring assetPath = m_editor->getSettings()->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, asset->getFileName());

	Ref< IStream > stream = FileSystem::getInstance().open(fileName, File::FmRead);
	if (!stream)
		return false;

	// Create video decoder.
	Ref< VideoDecoderTheora > decoder = new VideoDecoderTheora();
	if (!decoder->create(stream))
		return false;

	// Create preview render widget.
	m_renderWidget = new ui::Widget();
	if (!m_renderWidget->create(parent, ui::WsClientBorder))
		return false;

	m_renderWidget->addSizeEventHandler(ui::createMethodHandler(this, &VideoEditorPage::eventSize));
	m_renderWidget->addPaintEventHandler(ui::createMethodHandler(this, &VideoEditorPage::eventPaint));

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 0;
	desc.multiSample = 0;
	desc.waitVBlank = false;
	desc.nativeWindowHandle = m_renderWidget->getIWidget()->getSystemHandle();

	m_renderView = renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	m_resourceManager = new resource::ResourceManager(true);
	m_resourceManager->addFactory(
		new render::ShaderFactory(m_editor->getOutputDatabase(), renderSystem)
	);

	if (!m_resourceManager->bind(c_idShaderMovie, m_shader))
		return false;

	// Create video player.
	Ref< Video > video = new Video();
	if (!video->create(renderSystem, decoder))
		return false;

	m_video = video;
	m_timer.start();

	m_idleHandler = ui::createMethodHandler(this, &VideoEditorPage::eventIdle);
	ui::Application::getInstance()->addEventHandler(ui::EiIdle, m_idleHandler);

	return true;
}

void VideoEditorPage::destroy()
{
	if (m_idleHandler)
	{
		ui::Application::getInstance()->removeEventHandler(ui::EiIdle, m_idleHandler);
		m_idleHandler = 0;
	}

	safeDestroy(m_screenRenderer);

	if (m_renderView)
	{
		m_renderView->close();
		m_renderView = 0;
	}

	safeDestroy(m_renderWidget);
}

void VideoEditorPage::activate()
{
}

void VideoEditorPage::deactivate()
{
}

bool VideoEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool VideoEditorPage::handleCommand(const ui::Command& command)
{
	return false;
}

void VideoEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void VideoEditorPage::eventSize(ui::Event* event)
{
	if (!m_renderView)
		return;

	ui::SizeEvent* s = checked_type_cast< ui::SizeEvent*, false >(event);
	ui::Size sz = s->getSize();

	m_renderView->reset(sz.cx, sz.cy);
	m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));
}

void VideoEditorPage::eventPaint(ui::Event* event)
{
	if (!m_renderView)
		return;

	if (m_renderView->begin(render::EtCyclop))
	{
		const Color4f clearColor(0.8f, 0.8f, 0.8f, 0.0);
		m_renderView->clear(
			render::CfColor,
			&clearColor,
			0.0f,
			0
		);
		if (m_video)
		{
			render::ISimpleTexture* texture = m_video->getTexture();
			if (texture)
			{
				m_shader->setTextureParameter(L"Texture", texture);
				m_shader->setFloatParameter(L"Format", float(m_video->getFormat()));
				m_screenRenderer->draw(m_renderView, m_shader);
			}
		}
		m_renderView->end();
	}

	m_renderView->present();

	event->consume();
}

void VideoEditorPage::eventIdle(ui::Event* event)
{
	if (!m_video)
		return;

	ui::IdleEvent* idleEvent = checked_type_cast< ui::IdleEvent* >(event);
	if (m_renderWidget->isVisible(true))
	{
		float deltaTime = float(m_timer.getDeltaTime());
		m_video->update(deltaTime);
		m_renderWidget->update();
		idleEvent->requestMore();
	}
}

	}
}
