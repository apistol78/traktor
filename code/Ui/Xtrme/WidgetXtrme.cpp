#include "Ui/Xtrme/WidgetXtrme.h"
#include "Ui/Xtrme/CanvasXtrme.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/Events/PaintEvent.h"
#include "Ui/Itf/IWidget.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"

namespace traktor
{
	namespace ui
	{
		namespace xtrme
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.xtrme.WidgetXtrme", WidgetXtrme, Widget)

Ref< render::IRenderSystem > WidgetXtrme::ms_renderSystem;

bool WidgetXtrme::create(Widget* parent, int style)
{
	T_ASSERT_M (ms_renderSystem, L"No render system attached");

	style &= ~(WsAccelerated | WsDoubleBuffer);

	if (!Widget::create(parent, style))
		return false;

	render::RenderViewCreateEmbeddedDesc desc;
	desc.depthBits = 0;
	desc.stencilBits = 0;
	desc.multiSample = 0;
	desc.waitVBlank = false;
	desc.nativeWindowHandle = getIWidget()->getSystemHandle();

	m_renderView = ms_renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_canvasImpl = new CanvasXtrme(ms_renderSystem, m_renderView, getFont());

	addSizeEventHandler(createMethodHandler(this, &WidgetXtrme::eventSize));
	addPaintEventHandler(createMethodHandler(this, &WidgetXtrme::eventPaint));

	return true;
}

void WidgetXtrme::destroy()
{
	if (m_canvasImpl)
	{
		delete m_canvasImpl;
		m_canvasImpl = 0;
	}

	if (m_renderView)
	{
		m_renderView->close();
		m_renderView = 0;
	}

	Widget::destroy();
}

void WidgetXtrme::addPaintXtrmeEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiPaintXtrme, eventHandler);
}

void WidgetXtrme::setRenderSystem(render::IRenderSystem* renderSystem)
{
	ms_renderSystem = renderSystem;
}

void WidgetXtrme::eventSize(Event* event)
{
	if (!m_renderView)
		return;

	Size size = checked_type_cast< SizeEvent* >(event)->getSize();
	m_renderView->resize(size.cx, size.cy);
}

void WidgetXtrme::eventPaint(Event* event)
{
	if (!m_renderView)
		return;

	if (!m_renderView->begin())
		return;

	const float color[] = { 0.88f, 0.88f, 0.89f, 0.0f };
	m_renderView->clear(render::CfColor, color, 1.0f, 0);

	m_canvasImpl->begin();

	Canvas canvas(m_canvasImpl);
	PaintEvent paintEvent(this, 0, canvas, static_cast< PaintEvent* >(event)->getUpdateRect());
	
	raiseEvent(EiPaintXtrme, &paintEvent);
	
	m_canvasImpl->end(
		getInnerRect().getSize()
	);

	m_renderView->end();
	m_renderView->present();

	event->consume();
}

		}
	}
}
