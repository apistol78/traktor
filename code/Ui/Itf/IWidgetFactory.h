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

class IButton;
class ICheckBox;
class IComboBox;
class IContainer;
class IDialog;
class IDropDown;
class IEdit;
class IFileDialog;
class IForm;
class IListBox;
class IListView;
class IMenuBar;
class IMessageBox;
class INotificationIcon;
class IPanel;
class IPathDialog;
class IPopupMenu;
class IRadioButton;
class IRichEdit;
class IScrollBar;
class ISlider;
class IStatic;
class IToolForm;
class ITreeView;
class IUserWidget;
class IWebBrowser;
class INative;
class ISystemBitmap;
class IClipboard;

/*! \brief Widget factory interface.
 * \ingroup UI
 */
class IWidgetFactory
{
public:
	virtual ~IWidgetFactory() {}

	virtual IButton* createButton(EventSubject* owner) = 0;

	virtual ICheckBox* createCheckBox(EventSubject* owner) = 0;

	virtual IComboBox* createComboBox(EventSubject* owner) = 0;

	virtual IContainer* createContainer(EventSubject* owner) = 0;

	virtual IDialog* createDialog(EventSubject* owner) = 0;

	virtual IDropDown* createDropDown(EventSubject* owner) = 0;

	virtual IEdit* createEdit(EventSubject* owner) = 0;

	virtual IFileDialog* createFileDialog(EventSubject* owner) = 0;

	virtual IForm* createForm(EventSubject* owner) = 0;

	virtual IListBox* createListBox(EventSubject* owner) = 0;

	virtual IListView* createListView(EventSubject* owner) = 0;

	virtual IMenuBar* createMenuBar(EventSubject* owner) = 0;

	virtual IMessageBox* createMessageBox(EventSubject* owner) = 0;

	virtual INotificationIcon* createNotificationIcon(EventSubject* owner) = 0;

	virtual IPanel* createPanel(EventSubject* owner) = 0;

	virtual IPathDialog* createPathDialog(EventSubject* owner) = 0;

	virtual IPopupMenu* createPopupMenu(EventSubject* owner) = 0;

	virtual IRadioButton* createRadioButton(EventSubject* owner) = 0;

	virtual IRichEdit* createRichEdit(EventSubject* owner) = 0;

	virtual IScrollBar* createScrollBar(EventSubject* owner) = 0;

	virtual ISlider* createSlider(EventSubject* owner) = 0;

	virtual IStatic* createStatic(EventSubject* owner) = 0;

	virtual IToolForm* createToolForm(EventSubject* owner) = 0;

	virtual ITreeView* createTreeView(EventSubject* owner) = 0;

	virtual IUserWidget* createUserWidget(EventSubject* owner) = 0;

	virtual IWebBrowser* createWebBrowser(EventSubject* owner) = 0;

	virtual INative* createNative(EventSubject* owner) = 0;

	virtual ISystemBitmap* createBitmap() = 0;

	virtual IClipboard* createClipboard() = 0;

	virtual int32_t getSystemDPI() const = 0;

	virtual bool getSystemColor(SystemColor systemColor, Color4ub& outColor) = 0;

	virtual void getSystemFonts(std::list< std::wstring >& outFonts) = 0;
};

	}
}

#endif	// traktor_ui_IWidgetFactory_H
