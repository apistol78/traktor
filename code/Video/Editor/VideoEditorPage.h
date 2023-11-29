/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Timer/Timer.h"
#include "Editor/IEditorPage.h"
#include "Resource/Proxy.h"
#include "Ui/EventSubject.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_VIDEO_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::editor
{

class IDocument;
class IEditor;
class IEditorPageSite;

}

namespace traktor::render
{

class IRenderView;
class ScreenRenderer;
class Shader;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::ui
{

class IdleEvent;
class PaintEvent;
class SizeEvent;
class Widget;

}

namespace traktor::video
{

class Video;
class VideoAsset;

/*!
 * \ingroup Video
 */
class T_DLLCLASS VideoEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	explicit VideoEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< ui::EventSubject::IEventHandler > m_idleEventHandler;
	Ref< ui::Widget > m_renderWidget;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderView > m_renderView;
	Ref< render::ScreenRenderer > m_screenRenderer;
	resource::Proxy< render::Shader > m_shader;
	Ref< Video > m_video;
	Timer m_timer;

	void eventSize(ui::SizeEvent* event);

	void eventPaint(ui::PaintEvent* event);

	void eventIdle(ui::IdleEvent* event);
};

}
