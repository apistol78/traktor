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
class MoviePlayer;

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

class WidgetPreviewControl : public ui::Widget
{
	T_RTTI_CLASS;

public:
	WidgetPreviewControl(editor::IEditor* editor, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem);

	bool create(ui::Widget* parent);

	virtual void destroy() T_OVERRIDE T_FINAL;

	void setScaffoldingClass(const IRuntimeClass* scaffoldingClass);

private:
	editor::IEditor* m_editor;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	//Ref< db::Database > m_database;
	Ref< render::IRenderView > m_renderView;
	Ref< flash::AccDisplayRenderer > m_displayRenderer;
	Ref< flash::MoviePlayer > m_moviePlayer;
	Ref< ui::EventSubject::IEventHandler > m_idleEventHandler;
	Timer m_timer;

	Ref< const IRuntimeClass > m_scaffoldingClass;
	Ref< ITypedObject > m_scaffolding;

	void eventSize(ui::SizeEvent* event);

	void eventPaint(ui::PaintEvent* event);

	void eventIdle(ui::IdleEvent* event);
};

	}
}

#endif	// traktor_uikit_WidgetPreviewControl_H
