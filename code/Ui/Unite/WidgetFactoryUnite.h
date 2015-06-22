#ifndef traktor_ui_WidgetFactoryUnite_H
#define traktor_ui_WidgetFactoryUnite_H

#include "Core/Misc/AutoPtr.h"
#include "Ui/Itf/IWidgetFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_UNITE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIUnite
 */
class T_DLLCLASS WidgetFactoryUnite : public IWidgetFactory
{
public:
	WidgetFactoryUnite(IWidgetFactory* nativeFactory);

	virtual IButton* createButton(IEventSubject* owner);

	virtual ICheckBox* createCheckBox(IEventSubject* owner);

	virtual IComboBox* createComboBox(IEventSubject* owner);

	virtual IContainer* createContainer(IEventSubject* owner);

	virtual IDialog* createDialog(IEventSubject* owner);

	virtual IDropDown* createDropDown(IEventSubject* owner);

	virtual IEdit* createEdit(IEventSubject* owner);

	virtual IFileDialog* createFileDialog(IEventSubject* owner);

	virtual IForm* createForm(IEventSubject* owner);

	virtual IListBox* createListBox(IEventSubject* owner);

	virtual IListView* createListView(IEventSubject* owner);

	virtual IMenuBar* createMenuBar(IEventSubject* owner);

	virtual IMessageBox* createMessageBox(IEventSubject* owner);

	virtual INotificationIcon* createNotificationIcon(IEventSubject* owner);

	virtual IPanel* createPanel(IEventSubject* owner);

	virtual IPopupMenu* createPopupMenu(IEventSubject* owner);

	virtual IRadioButton* createRadioButton(IEventSubject* owner);

	virtual IRichEdit* createRichEdit(IEventSubject* owner);

	virtual IScrollBar* createScrollBar(IEventSubject* owner);

	virtual ISlider* createSlider(IEventSubject* owner);

	virtual IStatic* createStatic(IEventSubject* owner);

	virtual IToolForm* createToolForm(IEventSubject* owner);

	virtual ITreeView* createTreeView(IEventSubject* owner);

	virtual IUserWidget* createUserWidget(IEventSubject* owner);

	virtual IWebBrowser* createWebBrowser(IEventSubject* owner);

	virtual INative* createNative(IEventSubject* owner);

	virtual IBitmap* createBitmap();

	virtual IClipboard* createClipboard();

	virtual int32_t getSystemDPI() const;

	virtual bool getSystemColor(SystemColor systemColor, Color4ub& outColor);

private:
	AutoPtr< IWidgetFactory > m_nativeFactory;
};

	}
}

#endif	// traktor_ui_WidgetFactoryUnite_H
