#ifndef traktor_ui_WidgetFactoryX11_H
#define traktor_ui_WidgetFactoryX11_H

#include <X11/Xlib.h>
#include "Ui/Itf/IWidgetFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_X11_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class T_DLLCLASS WidgetFactoryX11 : public IWidgetFactory
{
public:
	WidgetFactoryX11();

	virtual ~WidgetFactoryX11();

	virtual IEventLoop* createEventLoop(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IContainer* createContainer(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IDialog* createDialog(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IForm* createForm(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual INotificationIcon* createNotificationIcon(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IPathDialog* createPathDialog(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IToolForm* createToolForm(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IUserWidget* createUserWidget(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IWebBrowser* createWebBrowser(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual ISystemBitmap* createBitmap() T_OVERRIDE T_FINAL;

	virtual IClipboard* createClipboard() T_OVERRIDE T_FINAL;

	virtual int32_t getSystemDPI() const T_OVERRIDE T_FINAL;

	virtual void getSystemFonts(std::list< std::wstring >& outFonts) T_OVERRIDE T_FINAL;

	virtual void getDesktopRects(std::list< Rect >& outRects) const T_OVERRIDE T_FINAL;

private:
	Display* m_display;
	int32_t m_screen;
	XIM m_xim;
	int32_t m_dpi;
};

	}
}

#endif	// traktor_ui_WidgetFactoryX11_H

