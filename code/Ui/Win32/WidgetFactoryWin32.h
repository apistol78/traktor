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

	virtual IButton* createButton(EventSubject* owner);

	virtual ICheckBox* createCheckBox(EventSubject* owner);

	virtual IComboBox* createComboBox(EventSubject* owner);

	virtual IContainer* createContainer(EventSubject* owner);

	virtual IDialog* createDialog(EventSubject* owner);

	virtual IDropDown* createDropDown(EventSubject* owner);

	virtual IEdit* createEdit(EventSubject* owner);

	virtual IFileDialog* createFileDialog(EventSubject* owner);

	virtual IForm* createForm(EventSubject* owner);

	virtual IListBox* createListBox(EventSubject* owner);

	virtual IListView* createListView(EventSubject* owner);

	virtual IMenuBar* createMenuBar(EventSubject* owner);

	virtual IMessageBox* createMessageBox(EventSubject* owner);

	virtual INotificationIcon* createNotificationIcon(EventSubject* owner);

	virtual IPanel* createPanel(EventSubject* owner);

	virtual IPathDialog* createPathDialog(EventSubject* owner);

	virtual IPopupMenu* createPopupMenu(EventSubject* owner);

	virtual IRadioButton* createRadioButton(EventSubject* owner);

	virtual IRichEdit* createRichEdit(EventSubject* owner);

	virtual IScrollBar* createScrollBar(EventSubject* owner);

	virtual ISlider* createSlider(EventSubject* owner);

	virtual IStatic* createStatic(EventSubject* owner);

	virtual IToolForm* createToolForm(EventSubject* owner);

	virtual ITreeView* createTreeView(EventSubject* owner);

	virtual IUserWidget* createUserWidget(EventSubject* owner);

	virtual IWebBrowser* createWebBrowser(EventSubject* owner);

	virtual INative* createNative(EventSubject* owner);

	virtual IBitmap* createBitmap();

	virtual IClipboard* createClipboard();

	virtual int32_t getSystemDPI() const;

	virtual bool getSystemColor(SystemColor systemColor, Color4ub& outColor);

	virtual void getSystemFonts(std::list< std::wstring >& outFonts);

private:
	int32_t m_systemDPI;
};

	}
}

#endif	// traktor_ui_WidgetFactoryWin32_H
