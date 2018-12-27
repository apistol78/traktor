/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_WidgetFactoryWin32_H
#define traktor_ui_WidgetFactoryWin32_H

#include "Ui/Itf/IWidgetFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_WIN32_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class T_DLLCLASS WidgetFactoryWin32 : public IWidgetFactory
{
public:
	WidgetFactoryWin32();

	virtual IEventLoop* createEventLoop(EventSubject* owner) override final;

	virtual IContainer* createContainer(EventSubject* owner) override final;

	virtual IDialog* createDialog(EventSubject* owner) override final;

	virtual IForm* createForm(EventSubject* owner) override final;

	virtual INotificationIcon* createNotificationIcon(EventSubject* owner) override final;

	virtual IPathDialog* createPathDialog(EventSubject* owner) override final;

	virtual IToolForm* createToolForm(EventSubject* owner) override final;

	virtual IUserWidget* createUserWidget(EventSubject* owner) override final;

	virtual IWebBrowser* createWebBrowser(EventSubject* owner) override final;

	virtual ISystemBitmap* createBitmap() override final;

	virtual IClipboard* createClipboard() override final;

	virtual int32_t getSystemDPI() const override final;

	virtual void getSystemFonts(std::list< std::wstring >& outFonts) override final;

	virtual void getDesktopRects(std::list< Rect >& outRects) const override final;

private:
	int32_t m_systemDPI;
};

	}
}

#endif	// traktor_ui_WidgetFactoryWin32_H
