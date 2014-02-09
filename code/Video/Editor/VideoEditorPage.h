#ifndef traktor_video_VideoEditorPage_H
#define traktor_video_VideoEditorPage_H

#include "Core/Timer/Timer.h"
#include "Editor/IEditorPage.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_VIDEO_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IDocument;
class IEditor;
class IEditorPageSite;

	}

	namespace render
	{

class IRenderView;
class ScreenRenderer;
class Shader;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace ui
	{

class Event;
class EventHandler;
class Widget;

	}

	namespace video
	{

class Video;
class VideoAsset;

/*! \brief
 * \ingroup Video
 */
class T_DLLCLASS VideoEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	VideoEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent);

	virtual void destroy();

	virtual void activate();

	virtual void deactivate();

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position);

	virtual bool handleCommand(const ui::Command& command);

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId);

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< ui::Widget > m_renderWidget;
	Ref< ui::EventHandler > m_idleHandler;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderView > m_renderView;
	Ref< render::ScreenRenderer > m_screenRenderer;
	resource::Proxy< render::Shader > m_shader;
	Ref< Video > m_video;
	Timer m_timer;

	void eventSize(ui::Event* event);

	void eventPaint(ui::Event* event);

	void eventIdle(ui::Event* event);
};

	}
}

#endif	// traktor_video_VideoEditorPage_H
