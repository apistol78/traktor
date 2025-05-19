/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Ref.h"
#include "Core/Thread/Event.h"
#include "Core/Timer/Timer.h"
#include "Resource/Proxy.h"
#include "Ui/Widget.h"

namespace traktor
{

class IRuntimeClass;
class Thread;

}

namespace traktor::db
{

class Database;

}

namespace traktor::editor
{

class IEditor;

}

namespace traktor::spark
{

class AccDisplayRenderer;
class Movie;
class MoviePlayer;
class MovieRenderer;
class WireDisplayRenderer;

}

namespace traktor::render
{

class IRenderSystem;
class IRenderView;
class RenderContext;
class RenderGraph;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::uikit
{

class Scaffolding;

class PreviewControl : public ui::Widget
{
	T_RTTI_CLASS;

public:
	explicit PreviewControl(editor::IEditor* editor);

	bool create(ui::Widget* parent);

	virtual void destroy() override final;

	void setScaffolding(const Scaffolding* scaffolding);

	void setDebugWires(bool debugWires);

	bool getDebugWires() const { return m_debugWires; }

	void handleDatabaseEvent(db::Database* database, const Guid& eventId);

private:
	editor::IEditor* m_editor;
	Ref< const Scaffolding > m_scaffolding;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::IRenderView > m_renderView;
	Ref< render::RenderContext > m_renderContext;
	Ref< render::RenderGraph > m_renderGraph;
	Ref< spark::AccDisplayRenderer > m_displayRenderer;
	Ref< spark::WireDisplayRenderer > m_displayRendererWire;
	Ref< spark::MovieRenderer > m_movieRenderer;
	Ref< spark::MovieRenderer > m_movieRendererWire;
	Ref< spark::MoviePlayer > m_moviePlayer;
	Ref< ui::EventSubject::IEventHandler > m_idleEventHandler;
	Ref< spark::Movie > m_movie;
	resource::Proxy< IRuntimeClass > m_scaffoldingClass;
	Ref< ITypedObject > m_scaffoldingObject;
	bool m_debugWires = false;
	bool m_resized = false;
	Timer m_timer;

	Thread* m_threadProcessTicks = nullptr;
	Event m_eventProcessTickStart;
	Event m_eventProcessTickDone;

	void threadProcessTicks();

	bool processTick();

	void eventSize(ui::SizeEvent* event);

	void eventPaint(ui::PaintEvent* event);

	void eventIdle(ui::IdleEvent* event);

	void eventKey(ui::KeyEvent* event);

	void eventKeyDown(ui::KeyDownEvent* event);

	void eventKeyUp(ui::KeyUpEvent* event);

	void eventButtonDown(ui::MouseButtonDownEvent* event);

	void eventButtonUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventMouseWheel(ui::MouseWheelEvent* event);
};

}
