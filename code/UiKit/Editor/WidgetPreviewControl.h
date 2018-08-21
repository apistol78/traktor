#ifndef traktor_uikit_WidgetPreviewControl_H
#define traktor_uikit_WidgetPreviewControl_H

#include "Core/Ref.h"
#include "Core/Timer/Timer.h"
#include "Ui/Widget.h"

namespace traktor
{

class IRuntimeClass;

	namespace db
	{

class Database;

	}

	namespace editor
	{

class IEditor;

	}

	namespace flash
	{

class AccDisplayRenderer;
class Movie;
class MoviePlayer;
class MovieRenderer;
class WireDisplayRenderer;

	}

	namespace render
	{

class IRenderSystem;
class IRenderView;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace uikit
	{

class WidgetScaffolding;

class WidgetPreviewControl : public ui::Widget
{
	T_RTTI_CLASS;

public:
	WidgetPreviewControl(editor::IEditor* editor, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem);

	bool create(ui::Widget* parent);

	virtual void destroy() T_OVERRIDE T_FINAL;

	void setScaffolding(const WidgetScaffolding* scaffolding);

private:
	editor::IEditor* m_editor;
	Ref< const WidgetScaffolding > m_scaffolding;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::IRenderView > m_renderView;
	Ref< flash::AccDisplayRenderer > m_displayRenderer;
	Ref< flash::WireDisplayRenderer > m_displayRendererWire;
	Ref< flash::MovieRenderer > m_movieRenderer;
	Ref< flash::MovieRenderer > m_movieRendererWire;
	Ref< flash::MoviePlayer > m_moviePlayer;
	Ref< ui::EventSubject::IEventHandler > m_idleEventHandler;
	Ref< flash::Movie > m_movie;
	resource::Proxy< IRuntimeClass > m_scaffoldingClass;
	Ref< ITypedObject > m_scaffoldingObject;
	Timer m_timer;

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
}

#endif	// traktor_uikit_WidgetPreviewControl_H
