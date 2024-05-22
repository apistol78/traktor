/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Editor/IEditor.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Spark/DefaultCharacterFactory.h"
#include "Spark/Movie.h"
#include "Spark/MovieLoader.h"
#include "Spark/MoviePlayer.h"
#include "Spark/MovieRenderer.h"
#include "Spark/Frame.h"
#include "Spark/Key.h"
#include "Spark/Sprite.h"
#include "Spark/Acc/AccDisplayRenderer.h"
#include "Spark/Editor/PreviewControl.h"
#include "Spark/Sound/SoundRenderer.h"
#include "Spark/Sw/SwDisplayRenderer.h"
#include "Sound/Player/SoundPlayer.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Application.h"

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
	{ ui::VkBackSpace, Key::AkBackspace },
	{ ui::VkControl, Key::AkControl },
	{ ui::VkDelete, Key::AkDeleteKey },
	{ ui::VkDown, Key::AkDown },
	{ ui::VkEnd, Key::AkEnd },
	{ ui::VkReturn, Key::AkEnter },
	{ ui::VkEscape, Key::AkEscape },
	{ ui::VkHome, Key::AkHome },
	{ ui::VkInsert, Key::AkInsert },
	{ ui::VkLeft, Key::AkLeft },
	{ ui::VkPageDown, Key::AkPgDn },
	{ ui::VkPageUp, Key::AkPgUp },
	{ ui::VkRight, Key::AkRight },
	{ ui::VkShift, Key::AkShift },
	{ ui::VkSpace, Key::AkSpace },
	{ ui::VkTab, Key::AkTab },
	{ ui::VkUp, Key::AkUp },

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
	if (!Widget::create(parent, style | ui::WsNoCanvas))
		return false;

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 8;
	desc.multiSample = m_editor->getSettings()->getProperty< int32_t >(L"Editor.MultiSample", 4);
	desc.multiSampleShading = m_editor->getSettings()->getProperty< float >(L"Editor.MultiSampleShading", 0.0f);
	desc.waitVBlanks = 1;
	desc.syswin = getIWidget()->getSystemWindow();

	m_renderView = renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_renderContext = new render::RenderContext(4 * 1024 * 1024);
	m_renderGraph = new render::RenderGraph(renderSystem, desc.multiSample);

	m_displayRenderer = new AccDisplayRenderer();
	m_displayRenderer->create(
		resourceManager,
		renderSystem,
		1,
		true
	);

	m_movieRenderer = new MovieRenderer(m_displayRenderer);

	if (soundPlayer)
	{
		m_soundRenderer = new SoundRenderer();
		m_soundRenderer->create(soundPlayer);
	}
	else
		log::warning << L"Unable to create audio system; all sound disabled." << Endl;

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
	m_timer.reset();
	return true;
}

void PreviewControl::destroy()
{
	ui::Application::getInstance()->removeEventHandler(m_idleEventHandler);

	safeDestroy(m_moviePlayer);
	safeDestroy(m_soundRenderer);
	safeDestroy(m_displayRenderer);
	safeDestroy(m_renderGraph);
	safeClose(m_renderView);

	m_movieRenderer = nullptr;

	Widget::destroy();
}

void PreviewControl::setMovie(Movie* movie)
{
	m_movie = movie;

	if (m_moviePlayer)
		m_moviePlayer->destroy();

	const ui::Size sz = getInnerRect().getSize();

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

ui::Size PreviewControl::getPreferredSize(const ui::Size& hint) const
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

	if (m_renderView)
		m_renderView->reset(sz.cx, sz.cy);

	if (m_moviePlayer)
		m_moviePlayer->postViewResize(sz.cx, sz.cy);
}

void PreviewControl::eventPaint(ui::PaintEvent* event)
{
	if (!m_renderView)
		return;

	ui::Size sz = getInnerRect().getSize();

	// Render view events; reset view if it has become lost.
	render::RenderEvent re;
	while (m_renderView->nextEvent(re))
	{
		if (re.type == render::RenderEventType::Lost)
			m_renderView->reset(sz.cx, sz.cy);
	}

	// Add passes to render graph.
	m_displayRenderer->beginSetup(m_renderGraph);
	m_moviePlayer->render(m_movieRenderer);
	m_displayRenderer->endSetup();

	// Validate render graph.
	if (!m_renderGraph->validate())
		return;

	// Build render context.
	m_renderContext->flush();
	m_renderGraph->build(m_renderContext, sz.cx, sz.cy);

	// Render frame.
	if (m_renderView->beginFrame())
	{
		m_renderContext->render(m_renderView);
		m_renderView->endFrame();
		m_renderView->present();
	}

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
				update();
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
