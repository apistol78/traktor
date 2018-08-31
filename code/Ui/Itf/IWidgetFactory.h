/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_IWidgetFactory_H
#define traktor_ui_IWidgetFactory_H

#include <list>
#include <string>
#include "Core/Config.h"
#include "Core/Math/Color4ub.h"
#include "Ui/Enums.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

class IContainer;
class IDialog;
class IFileDialog;
class IForm;
class IMessageBox;
class INotificationIcon;
class IPathDialog;
class IToolForm;
class IUserWidget;
class IWebBrowser;
class ISystemBitmap;
class IClipboard;

/*! \brief Widget factory interface.
 * \ingroup UI
 */
class IWidgetFactory
{
public:
	virtual ~IWidgetFactory() {}

	virtual IContainer* createContainer(EventSubject* owner) = 0;

	virtual IDialog* createDialog(EventSubject* owner) = 0;

	virtual IFileDialog* createFileDialog(EventSubject* owner) = 0;

	virtual IForm* createForm(EventSubject* owner) = 0;

	virtual IMessageBox* createMessageBox(EventSubject* owner) = 0;

	virtual INotificationIcon* createNotificationIcon(EventSubject* owner) = 0;

	virtual IPathDialog* createPathDialog(EventSubject* owner) = 0;

	virtual IToolForm* createToolForm(EventSubject* owner) = 0;

	virtual IUserWidget* createUserWidget(EventSubject* owner) = 0;

	virtual IWebBrowser* createWebBrowser(EventSubject* owner) = 0;

	virtual ISystemBitmap* createBitmap() = 0;

	virtual IClipboard* createClipboard() = 0;

	virtual int32_t getSystemDPI() const = 0;

	virtual bool getSystemColor(SystemColor systemColor, Color4ub& outColor) = 0;

	virtual void getSystemFonts(std::list< std::wstring >& outFonts) = 0;
};

	}
}

#endif	// traktor_ui_IWidgetFactory_H
