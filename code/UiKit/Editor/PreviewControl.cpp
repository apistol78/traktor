/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "UiKit/Editor/PreviewControl.h"

#include "Core/Class/IRuntimeClass.h"
#include "Core/Class/IRuntimeDispatch.h"
#include "Core/Log/Log.h"
#include "Core/Misc/ObjectStore.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Editor/IEditor.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/TextureFactory.h"
#include "Resource/ResourceManager.h"
#include "Script/Editor/ScriptDebuggerSessions.h"
#include "Script/IScriptManager.h"
#include "Script/ScriptChunk.h"
#include "Script/ScriptFactory.h"
#include "Spark/Acc/AccDisplayRenderer.h"
#include "Spark/Debug/WireDisplayRenderer.h"
#include "Spark/DefaultCharacterFactory.h"
#include "Spark/Frame.h"
#include "Spark/Key.h"
#include "Spark/Movie.h"
#include "Spark/MoviePlayer.h"
#include "Spark/MovieRenderer.h"
#include "Spark/MovieResourceFactory.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteInstance.h"
#include "Ui/Application.h"
#include "Ui/Itf/IWidget.h"
#include "UiKit/Editor/Scaffolding.h"
#include "Video/VideoFactory.h"

namespace traktor::uikit
{
namespace
{

const struct
{
	ui::VirtualKey vk;
	int32_t ak;
} c_askeys[] = {
	{ ui::VkBackSpace, spark::Key::AkBackspace },
	{ ui::VkControl, spark::Key::AkControl },
	{ ui::VkDelete, spark::Key::AkDeleteKey },
	{ ui::VkDown, spark::Key::AkDown },
	{ ui::VkEnd, spark::Key::AkEnd },
	{ ui::VkReturn, spark::Key::AkEnter },
	{ ui::VkEscape, spark::Key::AkEscape },
	{ ui::VkHome, spark::Key::AkHome },
	{ ui::VkInsert, spark::Key::AkInsert },
	{ ui::VkLeft, spark::Key::AkLeft },
	{ ui::VkPageDown, spark::Key::AkPgDn },
	{ ui::VkPageUp, spark::Key::AkPgUp },
	{ ui::VkRight, spark::Key::AkRight },
	{ ui::VkShift, spark::Key::AkShift },
	{ ui::VkSpace, spark::Key::AkSpace },
	{ ui::VkTab, spark::Key::AkTab },
	{ ui::VkUp, spark::Key::AkUp }
};

int32_t translateVirtualKey(ui::VirtualKey vk)
{
	for (int i = 0; i < sizeof_array(c_askeys); ++i)
		if (vk == c_askeys[i].vk)
			return c_askeys[i].ak;
	return 0;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.uikit.PreviewControl", PreviewControl, ui::Widget)

PreviewControl::PreviewControl(editor::IEditor* editor)
	: m_editor(editor)
	, m_debugWires(false)
{
}

bool PreviewControl::create(ui::Widget* parent)
{
	if (!ui::Widget::create(parent, ui::WsFocus | ui::WsNoCanvas))
		return false;

	Ref< db::Database > database = m_editor->getOutputDatabase();
	if (!database)
		return false;

	Ref< render::IRenderSystem > renderSystem = m_editor->getObjectStore()->get< render::IRenderSystem >();
	if (!renderSystem)
		return false;

	Ref< script::IScriptManager > scriptManager = m_editor->getObjectStore()->get< script::IScriptManager >();
	if (!scriptManager)
		return false;

	m_renderSystem = renderSystem;

	// Create render view.
	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 8;
	desc.multiSample = m_editor->getSettings()->getProperty< int32_t >(L"Editor.MultiSample", 4);
	desc.multiSampleShading = m_editor->getSettings()->getProperty< float >(L"Editor.MultiSampleShading", 0.0f);
	desc.allowHDR = m_editor->getSettings()->getProperty< bool >(L"Editor.HDR", true);
	desc.waitVBlanks = 1;
	desc.syswin = getIWidget()->getSystemWindow();

	m_renderView = m_renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_renderContext = new render::RenderContext(4 * 1024 * 1024);
	m_renderGraph = new render::RenderGraph(m_renderSystem, desc.multiSample);

	Ref< script::IScriptDebugger > debugger = scriptManager->createDebugger();
	Ref< script::IScriptProfiler > profiler = scriptManager->createProfiler();

	m_editor->getObjectStore()->get< script::ScriptDebuggerSessions >()->beginSession(debugger, profiler);

	// Create resource manager.
	m_resourceManager = new resource::ResourceManager(database, m_editor->getSettings()->getProperty< bool >(L"Resource.Verbose", false));
	m_resourceManager->addFactory(new render::ShaderFactory(renderSystem));
	m_resourceManager->addFactory(new render::TextureFactory(renderSystem, 0));
	m_resourceManager->addFactory(new script::ScriptFactory(scriptManager));
	m_resourceManager->addFactory(new spark::MovieResourceFactory());
	m_resourceManager->addFactory(new video::VideoFactory(renderSystem));

	// Create an empty flash movie.
	Ref< spark::Sprite > movieClip = new spark::Sprite(60);
	Ref< spark::Frame > frame = new spark::Frame();
	frame->changeBackgroundColor(Color4f(0.2f, 0.2f, 0.2f, 1.0f));
	movieClip->addFrame(frame);
	m_movie = new spark::Movie(
		Aabb2(Vector2(0.0f, 0.0f), Vector2(1280.0f * 20.0f, 720.0f * 20.0f)),
		movieClip);

	// Create flash display renderer.
	m_displayRenderer = new spark::AccDisplayRenderer();
	if (!m_displayRenderer->create(
			m_resourceManager,
			m_renderSystem,
			1,
			true))
		return false;

	m_displayRendererWire = new spark::WireDisplayRenderer();
	if (!m_displayRendererWire->create(
			m_resourceManager,
			m_renderSystem,
			1))
	{
		log::error << L"Unable to create wire renderer." << Endl;
		return false;
	}

	// Create separate movie renderer for each display renderer.
	m_movieRenderer = new spark::MovieRenderer(m_displayRenderer);
	m_movieRendererWire = new spark::MovieRenderer(m_displayRendererWire);

	// Create flash movie player.
	m_moviePlayer = new spark::MoviePlayer(
		new spark::DefaultCharacterFactory(),
		nullptr,
		nullptr);
	if (!m_moviePlayer->create(m_movie, 1280, 720, nullptr))
		return false;

	while (!m_moviePlayer->progress(1.0f / 60.0f, nullptr))
		;

	// Add widget event handler.
	addEventHandler< ui::SizeEvent >(this, &PreviewControl::eventSize);
	addEventHandler< ui::PaintEvent >(this, &PreviewControl::eventPaint);
	addEventHandler< ui::KeyEvent >(this, &PreviewControl::eventKey);
	addEventHandler< ui::KeyDownEvent >(this, &PreviewControl::eventKeyDown);
	addEventHandler< ui::KeyUpEvent >(this, &PreviewControl::eventKeyUp);
	addEventHandler< ui::MouseButtonDownEvent >(this, &PreviewControl::eventButtonDown);
	addEventHandler< ui::MouseButtonUpEvent >(this, &PreviewControl::eventButtonUp);
	addEventHandler< ui::MouseMoveEvent >(this, &PreviewControl::eventMouseMove);
	addEventHandler< ui::MouseWheelEvent >(this, &PreviewControl::eventMouseWheel);

	// Register our idle event handler.
	m_idleEventHandler = ui::Application::getInstance()->addEventHandler< ui::IdleEvent >(this, &PreviewControl::eventIdle);

	// Create process thread.
	m_threadProcessTicks = ThreadManager::getInstance().create([this]() {
		threadProcessTicks();
	});
	m_threadProcessTicks->start();

	m_timer.reset();
	return true;
}

void PreviewControl::destroy()
{
	m_threadProcessTicks->stop();
	ThreadManager::getInstance().destroy(m_threadProcessTicks);
	m_threadProcessTicks = nullptr;

	ui::Application::getInstance()->removeEventHandler(m_idleEventHandler);

	safeDestroy(m_displayRenderer);
	safeDestroy(m_renderGraph);
	safeClose(m_renderView);
	safeDestroy(m_resourceManager);

	ui::Widget::destroy();
}

void PreviewControl::setScaffolding(const Scaffolding* scaffolding)
{
	m_scaffoldingObject = nullptr;
	if ((m_scaffolding = scaffolding) != nullptr)
		m_resourceManager->bind(m_scaffolding->getScaffoldingClass(), m_scaffoldingClass);
	else
		m_scaffoldingClass.clear();
}

void PreviewControl::setDebugWires(bool debugWires)
{
	m_debugWires = debugWires;
}

void PreviewControl::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	if (m_resourceManager && database == m_editor->getOutputDatabase())
	{
		if (m_resourceManager->reload(eventId, false))
		{
			// Content in resource manager was reloaded; in case it's not the main scaffolding script
			// we need to invalidate it manually so the scaffolding is re-created properly.
			//
			// This will cause the tag in the proxy to be zero so the scaffolding will get re-created.
			if (m_scaffolding != nullptr && (Guid)m_scaffolding->getScaffoldingClass() != eventId)
				m_scaffoldingClass.unconsume();
		}
	}
}

void PreviewControl::threadProcessTicks()
{
	while (!m_threadProcessTicks->stopped())
	{
		if (!m_eventProcessTickStart.wait(100))
			continue;

		// Got process event.
		const ui::Size sz = getInnerRect().getSize();
		const float scale = std::max(dpi() / 96.0f, 1.0f);

		// Initialize scaffolding.
		if (m_scaffoldingClass.changed())
		{
			// Create scaffolding object.
			if (m_scaffoldingClass)
			{
				// Remove previous scaffolding.
				if (m_scaffoldingObject)
				{
					const IRuntimeDispatch* method = findRuntimeClassMethod(m_scaffoldingClass, "remove");
					if (method != nullptr)
						method->invoke(m_scaffoldingObject, 0, nullptr);
				}

				// Construct new scaffolding.
				const Any argv[] = {
					Any::fromObject(m_editor->getSourceDatabase()),
					Any::fromObject(m_resourceManager),
					Any::fromObject(m_moviePlayer->getMovieInstance()),
					Any::fromInt32((int32_t)(sz.cx / scale)),
					Any::fromInt32((int32_t)(sz.cy / scale))
				};
				m_scaffoldingObject = createRuntimeClassInstance(m_scaffoldingClass, nullptr, sizeof_array(argv), argv);
			}
			else
				m_scaffoldingObject = nullptr;

			m_scaffoldingClass.consume();
		}

		if (m_scaffoldingObject)
		{
			const IRuntimeDispatch* method = findRuntimeClassMethod(m_scaffoldingClass, "update");
			if (method != nullptr)
				method->invoke(m_scaffoldingObject, 0, nullptr);
		}

		if (m_moviePlayer)
		{
			if (m_resized)
			{
				m_moviePlayer->postViewResize((int32_t)(sz.cx / scale), (int32_t)(sz.cy / scale));
				m_resized = false;
			}

			const float deltaTime = (float)m_timer.getDeltaTime();
			m_moviePlayer->progress(deltaTime, nullptr);
		}

		m_eventProcessTickDone.broadcast();
	}
}

bool PreviewControl::processTick()
{
	m_eventProcessTickStart.broadcast();
	return m_eventProcessTickDone.wait(100);
}

void PreviewControl::eventSize(ui::SizeEvent* event)
{
	const ui::Size sz = getInnerRect().getSize();
	const float scale = std::max(dpi() / 96.0f, 1.0f);

	if (m_renderView)
		m_renderView->reset(sz.cx, sz.cy);

	m_resized = true;
	update();
}

void PreviewControl::eventPaint(ui::PaintEvent* event)
{
	if (!m_renderView || !m_moviePlayer)
		return;

	const ui::Size sz = getInnerRect().getSize();
	const float scale = std::max(dpi() / 96.0f, 1.0f);

	// Render view events; reset view if it has become lost.
	render::RenderEvent re;
	while (m_renderView->nextEvent(re))
		if (re.type == render::RenderEventType::Lost)
			m_renderView->reset(sz.cx, sz.cy);

	if (!processTick())
		log::debug << L"processTick timeout; logic stuck" << Endl;

	// Add passes to render graph.
	m_displayRenderer->beginSetup(m_renderGraph);
	m_moviePlayer->render(m_movieRenderer);
	m_displayRenderer->endSetup();

	// Render debug wires.
	if (m_debugWires)
	{
		Ref< render::RenderPass > rp = new render::RenderPass(L"Debug wire");
		rp->setOutput(render::RGTargetSet::Output, render::TfAll, render::TfAll);
		rp->addBuild([&](const render::RenderGraph&, render::RenderContext* renderContext) {
			m_displayRendererWire->begin(0);
			m_moviePlayer->render(m_movieRendererWire);
			m_displayRendererWire->end(0);

			auto rb = renderContext->allocNamed< render::LambdaRenderBlock >(L"Debug wire");
			rb->lambda = [&](render::IRenderView* renderView) {
				m_displayRendererWire->render(m_renderView, 0);
			};
			renderContext->draw(rb);
		});
		m_renderGraph->addPass(rp);
	}

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
	if (m_moviePlayer && isVisible(true))
		update();

	event->requestMore();
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
		const int32_t ak = translateVirtualKey(event->getVirtualKey());
		if (ak > 0)
			m_moviePlayer->postKeyDown(ak);
	}
}

void PreviewControl::eventKeyUp(ui::KeyUpEvent* event)
{
	if (m_moviePlayer)
	{
		const int32_t ak = translateVirtualKey(event->getVirtualKey());
		if (ak > 0)
			m_moviePlayer->postKeyUp(ak);
	}
}

void PreviewControl::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	if (m_moviePlayer)
	{
		const ui::Point mousePosition = event->getPosition();
		const float scale = std::max(dpi() / 96.0f, 1.0f);
		m_moviePlayer->postMouseDown(
			(int32_t)(mousePosition.x / scale),
			(int32_t)(mousePosition.y / scale),
			event->getButton());
	}
	setCapture();
	setFocus();
}

void PreviewControl::eventButtonUp(ui::MouseButtonUpEvent* event)
{
	if (m_moviePlayer)
	{
		const ui::Point mousePosition = event->getPosition();
		const float scale = std::max(dpi() / 96.0f, 1.0f);
		m_moviePlayer->postMouseUp(
			(int32_t)(mousePosition.x / scale),
			(int32_t)(mousePosition.y / scale),
			event->getButton());
	}
	releaseCapture();
}

void PreviewControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	if (m_moviePlayer)
	{
		const ui::Point mousePosition = event->getPosition();
		const float scale = std::max(dpi() / 96.0f, 1.0f);
		m_moviePlayer->postMouseMove(
			(int32_t)(mousePosition.x / scale),
			(int32_t)(mousePosition.y / scale),
			event->getButton());
	}
}

void PreviewControl::eventMouseWheel(ui::MouseWheelEvent* event)
{
	if (m_moviePlayer)
	{
		const ui::Point mousePosition = event->getPosition();
		const float scale = std::max(dpi() / 96.0f, 1.0f);
		m_moviePlayer->postMouseWheel(
			(int32_t)(mousePosition.x / scale),
			(int32_t)(mousePosition.y / scale),
			event->getRotation());
	}
}

}
