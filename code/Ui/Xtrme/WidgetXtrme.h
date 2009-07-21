#ifndef traktor_ui_xtrme_WidgetXtrme_H
#define traktor_ui_xtrme_WidgetXtrme_H

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_XTRME_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class IRenderView;

	}

	namespace ui
	{
		namespace xtrme
		{

class CanvasXtrme;

class T_DLLCLASS WidgetXtrme : public Widget
{
	T_RTTI_CLASS(WidgetXtrme)

public:
	enum EventId
	{
		EiPaintXtrme = EiUser
	};

	bool create(Widget* parent, int style);

	void destroy();

	void addPaintXtrmeEventHandler(EventHandler* eventHandler);

	static void setRenderSystem(render::IRenderSystem* renderSystem);

private:
	static Ref< render::IRenderSystem > ms_renderSystem;
	Ref< render::IRenderView > m_renderView;
	CanvasXtrme* m_canvasImpl;

	void eventSize(Event* event);

	void eventPaint(Event* event);
};

		}
	}
}

#endif	// traktor_ui_xtrme_WidgetXtrme_H
