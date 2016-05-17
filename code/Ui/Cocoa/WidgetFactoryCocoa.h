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
	virtual IButton* createButton(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual ICheckBox* createCheckBox(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IComboBox* createComboBox(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IContainer* createContainer(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IDialog* createDialog(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IDropDown* createDropDown(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IEdit* createEdit(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IFileDialog* createFileDialog(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IForm* createForm(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IListBox* createListBox(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IListView* createListView(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IMenuBar* createMenuBar(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IMessageBox* createMessageBox(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual INotificationIcon* createNotificationIcon(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IPanel* createPanel(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IPathDialog* createPathDialog(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IPopupMenu* createPopupMenu(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IRadioButton* createRadioButton(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IRichEdit* createRichEdit(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IScrollBar* createScrollBar(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual ISlider* createSlider(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IStatic* createStatic(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IToolForm* createToolForm(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual ITreeView* createTreeView(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IUserWidget* createUserWidget(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual IWebBrowser* createWebBrowser(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual INative* createNative(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual ISystemBitmap* createBitmap() T_OVERRIDE T_FINAL;

	virtual IClipboard* createClipboard() T_OVERRIDE T_FINAL;

	virtual int32_t getSystemDPI() const T_OVERRIDE T_FINAL;

	virtual bool getSystemColor(SystemColor systemColor, Color4ub& outColor) T_OVERRIDE T_FINAL;

	virtual void getSystemFonts(std::list< std::wstring >& outFonts) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_ui_WidgetFactoryCocoa_H
