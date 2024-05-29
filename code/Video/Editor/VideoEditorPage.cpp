/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Misc/ObjectStore.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Render/IProgram.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Resource/ShaderFactory.h"
#include "Resource/ResourceManager.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/Widget.h"
#include "Ui/Itf/IWidget.h"
#include "Video/Video.h"
#include "Video/Decoders/VideoDecoderTheora.h"
#include "Video/Editor/VideoAsset.h"
#include "Video/Editor/VideoEditorPage.h"

namespace traktor::video
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
	T_ASSERT(asset);

	render::IRenderSystem* renderSystem = m_editor->getObjectStore()->get< render::IRenderSystem >();
	if (!renderSystem)
		return false;

	// Open file stream to source video.
	std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
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
	if (!m_renderWidget->create(parent, ui::WsFocus | ui::WsNoCanvas))
		return false;

	m_renderWidget->addEventHandler< ui::SizeEvent >(this, &VideoEditorPage::eventSize);
	m_renderWidget->addEventHandler< ui::PaintEvent >(this, &VideoEditorPage::eventPaint);

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 0;
	desc.stencilBits = 0;
	desc.multiSample = 0;
	desc.waitVBlanks = 1;
	desc.syswin = m_renderWidget->getIWidget()->getSystemWindow();

	m_renderView = renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	m_resourceManager = new resource::ResourceManager(m_editor->getOutputDatabase(), m_editor->getSettings()->getProperty< bool >(L"Resource.Verbose", false));
	m_resourceManager->addFactory(new render::ShaderFactory(renderSystem));

	if (!m_resourceManager->bind(c_idShaderMovie, m_shader))
		return false;

	// Create video player.
	Ref< Video > video = new Video();
	if (!video->create(renderSystem, decoder))
		return false;

	m_video = video;
	m_timer.reset();

	m_idleEventHandler = ui::Application::getInstance()->addEventHandler< ui::IdleEvent >(this, &VideoEditorPage::eventIdle);

	return true;
}

void VideoEditorPage::destroy()
{
	ui::Application::getInstance()->removeEventHandler(m_idleEventHandler);
	safeDestroy(m_screenRenderer);
	safeClose(m_renderView);
	safeDestroy(m_renderWidget);
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

void VideoEditorPage::eventSize(ui::SizeEvent* event)
{
	if (!m_renderView)
		return;

	ui::Size sz = event->getSize();
	m_renderView->reset(sz.cx, sz.cy);
}

void VideoEditorPage::eventPaint(ui::PaintEvent* event)
{
	if (!m_renderView)
		return;

	if (!m_renderView->beginFrame())
		return;

	render::Clear cl;
	cl.mask = render::CfColor;
	cl.colors[0] = Color4f(0.8f, 0.8f, 0.8f, 0.0);

	if (m_renderView->beginPass(&cl, render::TfAll, render::TfAll))
	{
		if (m_video)
		{
			render::ITexture* texture = m_video->getTexture();
			if (texture)
			{
				auto program = m_shader->getProgram().program;
				program->setTextureParameter(render::getParameterHandle(L"Texture"), texture);
				program->setVectorParameter(render::getParameterHandle(L"Bounds"), Vector4(0.0f, 0.0f, 1.0f, 1.0f));
				m_screenRenderer->draw(m_renderView, program);
			}
		}
		m_renderView->endPass();
	}

	m_renderView->endFrame();
	m_renderView->present();

	event->consume();
}

void VideoEditorPage::eventIdle(ui::IdleEvent* event)
{
	if (!m_video)
		return;

	if (m_renderWidget->isVisible(true))
	{
		float deltaTime = float(m_timer.getDeltaTime());
		m_video->update(deltaTime);
		m_renderWidget->update();
		event->requestMore();
	}
}

}
