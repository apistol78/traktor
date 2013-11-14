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
#include "Flash/Action/Avm1/Classes/AsKey.h"
#include "Flash/Sound/SoundRenderer.h"
#include "Flash/Sw/SwDisplayRenderer.h"
#include "Sound/Player/SoundPlayer.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Application.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/IdleEvent.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/Events/KeyEvent.h"
#include "Ui/Events/MouseEvent.h"
#if T_USE_ACCELERATED_RENDERER
#	include "Render/IRenderSystem.h"
#	include "Render/IRenderView.h"
#	include "Render/ScreenRenderer.h"
#	include "Render/Shader.h"
#	include "Render/RenderTargetSet.h"
#else
#	include "Graphics/GraphicsSystem.h"
#	if defined(_WIN32)
#		include "Graphics/Dd7/GraphicsSystemDd7.h"
#	endif
#	include "Graphics/Surface.h"
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.FlashPreviewControl", FlashPreviewControl, ui::Widget)

FlashPreviewControl::FlashPreviewControl()
:	m_playing(false)
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
	desc.width = 16;
	desc.height = 16;
	desc.pixelFormat = graphics::PfeA8R8G8B8;

#if defined(_WIN32)
	m_graphicsSystem = new graphics::GraphicsSystemDd7();
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

	addSizeEventHandler(ui::createMethodHandler(this, &FlashPreviewControl::eventSize));
	addPaintEventHandler(ui::createMethodHandler(this, &FlashPreviewControl::eventPaint));
	addKeyEventHandler(ui::createMethodHandler(this, &FlashPreviewControl::eventKey));
	addKeyDownEventHandler(ui::createMethodHandler(this, &FlashPreviewControl::eventKeyDown));
	addKeyUpEventHandler(ui::createMethodHandler(this, &FlashPreviewControl::eventKeyUp));
	addButtonDownEventHandler(ui::createMethodHandler(this, &FlashPreviewControl::eventButtonDown));
	addButtonUpEventHandler(ui::createMethodHandler(this, &FlashPreviewControl::eventButtonUp));
	addMouseMoveEventHandler(ui::createMethodHandler(this, &FlashPreviewControl::eventMouseMove));
	addMouseWheelEventHandler(ui::createMethodHandler(this, &FlashPreviewControl::eventMouseWheel));

	// Register our event handler in case of message idle.
	m_idleHandler = ui::createMethodHandler(this, &FlashPreviewControl::eventIdle);
	ui::Application::getInstance()->addEventHandler(ui::EiIdle, m_idleHandler);

	m_database = database;
	m_timer.start();
	return true;
}

void FlashPreviewControl::destroy()
{
	if (m_idleHandler)
	{
		ui::Application::getInstance()->removeEventHandler(ui::EiIdle, m_idleHandler);
		m_idleHandler = 0;
	}

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

void FlashPreviewControl::eventSize(ui::Event* event)
{
	ui::SizeEvent* s = static_cast< ui::SizeEvent* >(event);
	ui::Size sz = s->getSize();

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

void FlashPreviewControl::eventPaint(ui::Event* event)
{
#if T_USE_ACCELERATED_RENDERER
	if (!m_renderView)
		return;

	if (m_renderView->begin(render::EtCyclop))
	{
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

		m_displayRenderer->render(m_renderView, 0, render::EtCyclop, Vector2(0.0f, 0.0f));

		m_renderView->end();
		m_renderView->present();
	}
	
#else
	if (!m_graphicsSystem)
		return;

	Ref< graphics::Surface > surface = m_graphicsSystem->getSecondarySurface();

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

void FlashPreviewControl::eventIdle(ui::Event* event)
{
	if (!m_moviePlayer)
		return;

	ui::IdleEvent* idleEvent = checked_type_cast< ui::IdleEvent* >(event);
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

		idleEvent->requestMore();
	}
}

void FlashPreviewControl::eventKey(ui::Event* event)
{
	ui::KeyEvent* keyEvent = checked_type_cast< ui::KeyEvent* >(event);
	if (keyEvent->getCharacter() != '\r' && m_moviePlayer)
		m_moviePlayer->postKey(keyEvent->getCharacter());
}

void FlashPreviewControl::eventKeyDown(ui::Event* event)
{
	ui::KeyEvent* keyEvent = checked_type_cast< ui::KeyEvent* >(event);
	if (m_moviePlayer)
	{
		int32_t ak = translateVirtualKey(keyEvent->getVirtualKey());
		if (ak > 0)
			m_moviePlayer->postKeyDown(ak);
	}
}

void FlashPreviewControl::eventKeyUp(ui::Event* event)
{
	ui::KeyEvent* keyEvent = checked_type_cast< ui::KeyEvent* >(event);
	if (m_moviePlayer)
	{
		int32_t ak = translateVirtualKey(keyEvent->getVirtualKey());
		if (ak > 0)
			m_moviePlayer->postKeyUp(ak);
	}
}

void FlashPreviewControl::eventButtonDown(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);
	if (m_moviePlayer)
	{
		ui::Point mousePosition = mouseEvent->getPosition();
		m_moviePlayer->postMouseDown(mousePosition.x, mousePosition.y, mouseEvent->getButton());
	}
	setCapture();
	setFocus();
}

void FlashPreviewControl::eventButtonUp(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);
	if (m_moviePlayer)
	{
		ui::Point mousePosition = mouseEvent->getPosition();
		m_moviePlayer->postMouseUp(mousePosition.x, mousePosition.y, mouseEvent->getButton());
	}
	releaseCapture();
}

void FlashPreviewControl::eventMouseMove(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);
	if (m_moviePlayer)
	{
		ui::Point mousePosition = mouseEvent->getPosition();
		m_moviePlayer->postMouseMove(mousePosition.x, mousePosition.y, mouseEvent->getButton());
	}
}

void FlashPreviewControl::eventMouseWheel(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);
	if (m_moviePlayer)
	{
		ui::Point mousePosition = mouseEvent->getPosition();
		m_moviePlayer->postMouseWheel(mousePosition.x, mousePosition.y, mouseEvent->getWheelRotation());
	}
}

	}
}
