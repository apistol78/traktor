/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_WidgetFactoryCocoa_H
#define traktor_ui_WidgetFactoryCocoa_H

#include "Ui/Itf/IWidgetFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_COCOA_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class T_DLLCLASS WidgetFactoryCocoa : public IWidgetFactory
{
public:
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
};

	}
}

#endif	// traktor_ui_WidgetFactoryCocoa_H
