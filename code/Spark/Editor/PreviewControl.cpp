#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Editor/IEditor.h"
#include "Spark/DefaultCharacterFactory.h"
#include "Spark/Movie.h"
#include "Spark/MovieLoader.h"
#include "Spark/MoviePlayer.h"
#include "Spark/MovieRenderer.h"
#include "Spark/Frame.h"
#include "Spark/Sprite.h"
#include "Spark/Acc/AccDisplayRenderer.h"
#include "Spark/Action/Common/Classes/AsKey.h"
#include "Spark/Editor/PreviewControl.h"
#include "Spark/Sound/SoundRenderer.h"
#include "Spark/Sw/SwDisplayRenderer.h"
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
	namespace spark
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.PreviewControl", PreviewControl, ui::Widget)

PreviewControl::PreviewControl(editor::IEditor* editor)
:	m_editor(editor)
,	m_playing(false)
{
}

bool PreviewControl::create(
	ui::Widget* parent,
	int style,
	db::Database* database,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	sound::ISoundPlayer* soundPlayer
)
{
	if (!Widget::create(parent, style))
		return false;

#if T_USE_ACCELERATED_RENDERER
	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 8;
	desc.multiSample = m_editor->getSettings()->getProperty< int32_t >(L"Editor.MultiSample", 4);
	desc.waitVBlanks = 0;
	desc.syswin = getIWidget()->getSystemWindow();

	m_renderView = renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	render::Viewport viewport = m_renderView->getViewport();

	m_displayRenderer = new AccDisplayRenderer();
	m_displayRenderer->create(
		resourceManager,
		renderSystem,
		1,
		16 * 1024 * 1024,
		true,
		false,
		false,
		0.0f
	);

	m_movieRenderer = new MovieRenderer(m_displayRenderer, nullptr);

#else
	graphics::CreateDesc desc;
	desc.syswin = getIWidget()->getSystemWindow();
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

	m_image = new drawing::Image(drawing::PixelFormat::getA8R8G8B8(), 16, 16);
	m_displayRenderer = new SwDisplayRenderer(m_image, true);
#endif

	if (soundPlayer)
	{
		m_soundRenderer = new SoundRenderer();
		m_soundRenderer->create(soundPlayer);
	}
	else
		log::warning << L"Unable to create sound system; Flash sound disabled" << Endl;

	addEventHandler< ui::SizeEvent >(this, &PreviewControl::eventSize);
	addEventHandler< ui::PaintEvent >(this, &PreviewControl::eventPaint);
	addEventHandler< ui::KeyEvent >(this, &PreviewControl::eventKey);
	addEventHandler< ui::KeyDownEvent >(this, &PreviewControl::eventKeyDown);
	addEventHandler< ui::KeyUpEvent >(this, &PreviewControl::eventKeyUp);
	addEventHandler< ui::MouseButtonDownEvent >(this, &PreviewControl::eventButtonDown);
	addEventHandler< ui::MouseButtonUpEvent >(this, &PreviewControl::eventButtonUp);
	addEventHandler< ui::MouseMoveEvent >(this, &PreviewControl::eventMouseMove);
	addEventHandler< ui::MouseWheelEvent >(this, &PreviewControl::eventMouseWheel);

	m_idleEventHandler = ui::Application::getInstance()->addEventHandler< ui::IdleEvent >(this, &PreviewControl::eventIdle);

	m_database = database;
	m_timer.start();
	return true;
}

void PreviewControl::destroy()
{
	ui::Application::getInstance()->removeEventHandler< ui::IdleEvent >(m_idleEventHandler);

	safeDestroy(m_moviePlayer);
	safeDestroy(m_soundRenderer);

#if T_USE_ACCELERATED_RENDERER
	safeDestroy(m_displayRenderer);
	safeClose(m_renderView);
#else
	safeDestroy(m_graphicsSystem);
#endif

	m_movieRenderer = 0;

	Widget::destroy();
}

void PreviewControl::setMovie(Movie* movie)
{
	m_movie = movie;

	if (m_moviePlayer)
		m_moviePlayer->destroy();

	ui::Size sz = getInnerRect().getSize();

	m_moviePlayer = new MoviePlayer(
		new DefaultCharacterFactory(),
		new MovieLoader(),
		nullptr
	);
	m_moviePlayer->create(movie, sz.cx, sz.cy, nullptr);

	m_playing = true;
}

void PreviewControl::rewind()
{
	if (m_playing)
		m_moviePlayer->gotoAndPlay(0);
	else
		m_moviePlayer->gotoAndStop(0);
}

void PreviewControl::play()
{
	m_playing = true;
}

void PreviewControl::stop()
{
	m_playing = false;
}

void PreviewControl::forward()
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

bool PreviewControl::playing() const
{
	return m_playing;
}

ui::Size PreviewControl::getPreferedSize() const
{
	if (!m_movie)
		return ui::Size(400, 300);

	Aabb2 bounds = m_movie->getFrameBounds();

	int width = int(bounds.mx.x / 20.0f);
	int height = int(bounds.mx.y / 20.0f);

	return ui::Size(width, height);
}

ui::Point PreviewControl::getTwips(const ui::Point& pt) const
{
	ui::Size innerSize = getInnerRect().getSize();

	float x = (pt.x * m_movie->getFrameBounds().mx.x) / float(innerSize.cx);
	float y = (pt.y * m_movie->getFrameBounds().mx.y) / float(innerSize.cy);

	return ui::Point(int(x), int(y));
}

void PreviewControl::eventSize(ui::SizeEvent* event)
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

	m_image = new drawing::Image(drawing::PixelFormat::getA8R8G8B8(), sz.cx, sz.cy);
	m_displayRenderer->setImage(m_image);

#endif

	if (m_moviePlayer)
		m_moviePlayer->postViewResize(sz.cx, sz.cy);
}

void PreviewControl::eventPaint(ui::PaintEvent* event)
{
#if T_USE_ACCELERATED_RENDERER
	if (!m_renderView)
		return;

	render::Clear cl;
	cl.mask = render::CfColor | render::CfDepth | render::CfStencil;
	cl.colors[0] = Color4f(0.8f, 0.8f, 0.8f, 0.0);
	cl.depth = 1.0f;
	cl.stencil = 0;

	if (m_renderView->begin(&cl))
	{
		// Build render context.
		if (m_movie)
		{
			m_displayRenderer->build(uint32_t(0));
			m_moviePlayer->render(m_movieRenderer);
		}

		// Flush render context.
		m_displayRenderer->render(m_renderView, 0, Vector2(0.0f, 0.0f), 1.0f);

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
		m_moviePlayer->renderFrame();

		const uint32_t* s = reinterpret_cast< const uint32_t* >(m_image->getData());
		uint32_t* d = reinterpret_cast< uint32_t* >(bits);

		for (uint32_t y = 0; y < desc.height; ++y)
		{
			std::memcpy(d, s, desc.width * 4);
			s += m_image->getWidth();
			d += desc.pitch / 4;
		}

		surface->unlock();
	}

	m_graphicsSystem->flip(false);
#endif

	event->consume();
}

void PreviewControl::eventIdle(ui::IdleEvent* event)
{
	if (!m_moviePlayer)
		return;

	if (isVisible(true))
	{
		float deltaTime = float(m_timer.getDeltaTime());

		if (m_playing)
		{
			if (m_moviePlayer->progress(deltaTime, m_soundRenderer))
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

void PreviewControl::eventKey(ui::KeyEvent* event)
{
	if (event->getCharacter() != '\r' && m_moviePlayer)
		m_moviePlayer->postKey(event->getCharacter());
}

void PreviewControl::eventKeyDown(ui::KeyDownEvent* event)
{
	if (m_moviePlayer)
	{
		int32_t ak = translateVirtualKey(event->getVirtualKey());
		if (ak > 0)
			m_moviePlayer->postKeyDown(ak);
	}
}

void PreviewControl::eventKeyUp(ui::KeyUpEvent* event)
{
	if (m_moviePlayer)
	{
		int32_t ak = translateVirtualKey(event->getVirtualKey());
		if (ak > 0)
			m_moviePlayer->postKeyUp(ak);
	}
}

void PreviewControl::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	if (m_moviePlayer)
	{
		ui::Point mousePosition = event->getPosition();
		m_moviePlayer->postMouseDown(mousePosition.x, mousePosition.y, event->getButton());
	}
	setCapture();
	setFocus();
}

void PreviewControl::eventButtonUp(ui::MouseButtonUpEvent* event)
{
	if (m_moviePlayer)
	{
		ui::Point mousePosition = event->getPosition();
		m_moviePlayer->postMouseUp(mousePosition.x, mousePosition.y, event->getButton());
	}
	releaseCapture();
}

void PreviewControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	if (m_moviePlayer)
	{
		ui::Point mousePosition = event->getPosition();
		m_moviePlayer->postMouseMove(mousePosition.x, mousePosition.y, event->getButton());
	}
}

void PreviewControl::eventMouseWheel(ui::MouseWheelEvent* event)
{
	if (m_moviePlayer)
	{
		ui::Point mousePosition = event->getPosition();
		m_moviePlayer->postMouseWheel(mousePosition.x, mousePosition.y, event->getRotation());
	}
}

	}
}
