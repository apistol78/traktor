#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Flash/Editor/FlashPreviewControl.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMovieLoader.h"
#include "Flash/FlashMoviePlayer.h"
#include "Flash/FlashFrame.h"
#include "Flash/FlashSprite.h"
#include "Flash/Acc/AccDisplayRenderer.h"
#include "Flash/Action/Common/Classes/AsKey.h"
#include "Flash/Sound/SoundRenderer.h"
#include "Flash/Sw/SwDisplayRenderer.h"
#include "Sound/Player/SoundPlayer.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Application.h"
#if T_USE_ACCELERATED_RENDERER
#	include "Render/IRenderSystem.h"
#	include "Render/IRenderView.h"
#	include "Render/ScreenRenderer.h"
#	include "Render/Shader.h"
#	include "Render/RenderTargetSet.h"
#else
#	include "Graphics/IGraphicsSystem.h"
#	if defined(_WIN32)
#		include "Graphics/Gdi/GraphicsSystemGdi.h"
#	endif
#	include "Graphics/ISurface.h"
#endif

namespace traktor
{
	namespace flash
	{
		namespace
		{

const int c_updateInterval = 30;

const struct
{
	ui::VirtualKey vk;
	int32_t ak;
}
c_askeys[] =
{
	{ ui::VkBackSpace, AsKey::AkBackspace },
	{ ui::VkControl, AsKey::AkControl },
	{ ui::VkDelete, AsKey::AkDeleteKey },
	{ ui::VkDown, AsKey::AkDown },
	{ ui::VkEnd, AsKey::AkEnd },
	{ ui::VkReturn, AsKey::AkEnter },
	{ ui::VkEscape, AsKey::AkEscape },
	{ ui::VkHome, AsKey::AkHome },
	{ ui::VkInsert, AsKey::AkInsert },
	{ ui::VkLeft, AsKey::AkLeft },
	{ ui::VkPageDown, AsKey::AkPgDn },
	{ ui::VkPageUp, AsKey::AkPgUp },
	{ ui::VkRight, AsKey::AkRight },
	{ ui::VkShift, AsKey::AkShift },
	{ ui::VkSpace, AsKey::AkSpace },
	{ ui::VkTab, AsKey::AkTab },
	{ ui::VkUp, AsKey::AkUp },

	{ ui::VkK, 75 },
	{ ui::VkL, 76 }
};

int32_t translateVirtualKey(ui::VirtualKey vk)
{
	for (int i = 0; i < sizeof_array(c_askeys); ++i)
	{
		if (vk == c_askeys[i].vk)
			return c_askeys[i].ak;
	}
	return 0;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashPreviewControl", FlashPreviewControl, ui::Widget)

FlashPreviewControl::FlashPreviewControl()
:	m_playing(false)
,	m_wireframe(false)
{
}

bool FlashPreviewControl::create(
	ui::Widget* parent,
	int style,
	db::Database* database,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	sound::SoundSystem* soundSystem
)
{
	if (!Widget::create(parent, style))
		return false;

#if T_USE_ACCELERATED_RENDERER
	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 8;
	desc.multiSample = 0;
	desc.waitVBlank = false;
	desc.nativeWindowHandle = getIWidget()->getSystemHandle();

	m_renderView = renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	render::Viewport viewport = m_renderView->getViewport();

	m_displayRenderer = new AccDisplayRenderer();
	m_displayRenderer->create(
		resourceManager,
		renderSystem,
		1,
		true,
		0.0f
	);
#else
	graphics::CreateDesc desc;

	desc.windowHandle = getIWidget()->getSystemHandle();
	desc.fullScreen = false;
	desc.displayMode.width = 16;
	desc.displayMode.height = 16;
	desc.displayMode.bits = 32;
	desc.pixelFormat = graphics::PfeA8R8G8B8;

#if defined(_WIN32)
	m_graphicsSystem = new graphics::GraphicsSystemGdi();
#endif

	if (!m_graphicsSystem->create(desc))
		return false;

	m_displayRenderer = new SwDisplayRenderer();
#endif

	if (soundSystem)
	{
		Ref< sound::SoundPlayer > soundPlayer = new sound::SoundPlayer();
		soundPlayer->create(soundSystem, 0);

		m_soundRenderer = new SoundRenderer();
		m_soundRenderer->create(soundPlayer);
	}
	else
		log::warning << L"Unable to create sound system; Flash sound disabled" << Endl;

	addEventHandler< ui::SizeEvent >(this, &FlashPreviewControl::eventSize);
	addEventHandler< ui::PaintEvent >(this, &FlashPreviewControl::eventPaint);
	addEventHandler< ui::KeyEvent >(this, &FlashPreviewControl::eventKey);
	addEventHandler< ui::KeyDownEvent >(this, &FlashPreviewControl::eventKeyDown);
	addEventHandler< ui::KeyUpEvent >(this, &FlashPreviewControl::eventKeyUp);
	addEventHandler< ui::MouseButtonDownEvent >(this, &FlashPreviewControl::eventButtonDown);
	addEventHandler< ui::MouseButtonUpEvent >(this, &FlashPreviewControl::eventButtonUp);
	addEventHandler< ui::MouseMoveEvent >(this, &FlashPreviewControl::eventMouseMove);
	addEventHandler< ui::MouseWheelEvent >(this, &FlashPreviewControl::eventMouseWheel);

	m_idleEventHandler = ui::Application::getInstance()->addEventHandler< ui::IdleEvent >(this, &FlashPreviewControl::eventIdle);

	m_database = database;
	m_timer.start();
	return true;
}

void FlashPreviewControl::destroy()
{
	ui::Application::getInstance()->removeEventHandler< ui::IdleEvent >(m_idleEventHandler);

	safeDestroy(m_moviePlayer);
	safeDestroy(m_soundRenderer);

#if T_USE_ACCELERATED_RENDERER
	safeDestroy(m_displayRenderer);

	if (m_renderView)
	{
		m_renderView->close();
		m_renderView = 0;
	}
#else
	safeDestroy(m_graphicsSystem);
#endif

	Widget::destroy();
}

void FlashPreviewControl::setMovie(FlashMovie* movie)
{
	m_movie = movie;
	
	if (m_moviePlayer)
		m_moviePlayer->destroy();

	ui::Size sz = getInnerRect().getSize();

	m_moviePlayer = new FlashMoviePlayer(
		m_displayRenderer,
		m_soundRenderer,
		new flash::FlashMovieLoader(m_database)
	);
	m_moviePlayer->create(movie, sz.cx, sz.cy);

	m_playing = true;
}

void FlashPreviewControl::rewind()
{
	if (m_playing)
		m_moviePlayer->gotoAndPlay(0);
	else
		m_moviePlayer->gotoAndStop(0);
}

void FlashPreviewControl::play()
{
	m_playing = true;
}

void FlashPreviewControl::stop()
{
	m_playing = false;
}

void FlashPreviewControl::forward()
{
	uint32_t lastFrame = m_moviePlayer->getFrameCount();
	if (lastFrame > 0)
	{
		if (m_playing)
			m_moviePlayer->gotoAndPlay(lastFrame - 1);
		else
			m_moviePlayer->gotoAndStop(lastFrame - 1);
	}
}

bool FlashPreviewControl::playing() const
{
	return m_playing;
}

void FlashPreviewControl::setWireFrame(bool wireframe)
{
	if (wireframe != m_wireframe)
	{
		m_wireframe = wireframe;
		m_displayRenderer->flushCaches();
	}
}

ui::Size FlashPreviewControl::getPreferedSize() const
{
	if (!m_movie)
		return ui::Size(400, 300);

	Aabb2 bounds = m_movie->getFrameBounds();

	int width = int(bounds.mx.x / 20.0f);
	int height = int(bounds.mx.y / 20.0f);

	return ui::Size(width, height);
}

ui::Point FlashPreviewControl::getTwips(const ui::Point& pt) const
{
	ui::Size innerSize = getInnerRect().getSize();

	float x = (pt.x * m_movie->getFrameBounds().mx.x) / float(innerSize.cx);
	float y = (pt.y * m_movie->getFrameBounds().mx.y) / float(innerSize.cy);

	return ui::Point(int(x), int(y));
}

void FlashPreviewControl::eventSize(ui::SizeEvent* event)
{
	ui::Size sz = event->getSize();

#if T_USE_ACCELERATED_RENDERER
	if (m_renderView)
	{
		m_renderView->reset(sz.cx, sz.cy);
		m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));
	}
#else
	if (!m_graphicsSystem)
		return;

	m_graphicsSystem->resize(
		sz.cx,
		sz.cy
	);
#endif

	if (m_moviePlayer)
		m_moviePlayer->postViewResize(sz.cx, sz.cy);
}

void FlashPreviewControl::eventPaint(ui::PaintEvent* event)
{
#if T_USE_ACCELERATED_RENDERER
	if (!m_renderView)
		return;

	if (m_renderView->begin(render::EtCyclop))
	{
		m_displayRenderer->setWireframe(m_wireframe);

		// Build render context.
		if (m_movie)
		{
			m_displayRenderer->build(uint32_t(0));
			m_moviePlayer->renderFrame();
		}

		// Flush render context.
		const Color4f clearColor(0.8f, 0.8f, 0.8f, 0.0);
		m_renderView->clear(
			render::CfColor | render::CfDepth | render::CfStencil,
			&clearColor,
			1.0f,
			0
		);

		m_displayRenderer->render(m_renderView, 0, render::EtCyclop, Vector2(0.0f, 0.0f), 1.0f);

		m_renderView->end();
		m_renderView->present();
	}
	
#else
	if (!m_graphicsSystem)
		return;

	Ref< graphics::ISurface > surface = m_graphicsSystem->getSecondarySurface();

	graphics::SurfaceDesc desc;
	void* bits = surface->lock(desc);
	if (bits)
	{
		m_displayRenderer->setRasterTarget(bits, desc.width, desc.height, desc.pitch);
		m_moviePlayer->renderFrame();
		surface->unlock();
	}

	m_graphicsSystem->flip(false);
#endif

	event->consume();
}

void FlashPreviewControl::eventIdle(ui::IdleEvent* event)
{
	if (!m_moviePlayer)
		return;

	if (isVisible(true))
	{
		float deltaTime = float(m_timer.getDeltaTime());

		if (m_playing)
		{
			if (m_moviePlayer->progressFrame(deltaTime))
			{
				std::string command, args;
				while (m_moviePlayer->getFsCommand(command, args))
					log::info << L"FSCommand \"" << mbstows(command) << L"\" \"" << mbstows(args) << L"\"" << Endl;

				update();
			}
		}

		event->requestMore();
	}
}

void FlashPreviewControl::eventKey(ui::KeyEvent* event)
{
	if (event->getCharacter() != '\r' && m_moviePlayer)
		m_moviePlayer->postKey(event->getCharacter());
}

void FlashPreviewControl::eventKeyDown(ui::KeyDownEvent* event)
{
	if (m_moviePlayer)
	{
		int32_t ak = translateVirtualKey(event->getVirtualKey());
		if (ak > 0)
			m_moviePlayer->postKeyDown(ak);
	}
}

void FlashPreviewControl::eventKeyUp(ui::KeyUpEvent* event)
{
	if (m_moviePlayer)
	{
		int32_t ak = translateVirtualKey(event->getVirtualKey());
		if (ak > 0)
			m_moviePlayer->postKeyUp(ak);
	}
}

void FlashPreviewControl::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	if (m_moviePlayer)
	{
		ui::Point mousePosition = event->getPosition();
		m_moviePlayer->postMouseDown(mousePosition.x, mousePosition.y, event->getButton());
	}
	setCapture();
	setFocus();
}

void FlashPreviewControl::eventButtonUp(ui::MouseButtonUpEvent* event)
{
	if (m_moviePlayer)
	{
		ui::Point mousePosition = event->getPosition();
		m_moviePlayer->postMouseUp(mousePosition.x, mousePosition.y, event->getButton());
	}
	releaseCapture();
}

void FlashPreviewControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	if (m_moviePlayer)
	{
		ui::Point mousePosition = event->getPosition();
		m_moviePlayer->postMouseMove(mousePosition.x, mousePosition.y, event->getButton());
	}
}

void FlashPreviewControl::eventMouseWheel(ui::MouseWheelEvent* event)
{
	if (m_moviePlayer)
	{
		ui::Point mousePosition = event->getPosition();
		m_moviePlayer->postMouseWheel(mousePosition.x, mousePosition.y, event->getRotation());
	}
}

	}
}
