#include "Ui/Itf/IUserWidget.h"
#include "Ui/Unite/EditUnite.h"
#include "Ui/Unite/WidgetFactoryUnite.h"

namespace traktor
{
	namespace ui
	{

WidgetFactoryUnite::WidgetFactoryUnite(IWidgetFactory* nativeFactory)
:	m_nativeFactory(nativeFactory)
{
}

IButton* WidgetFactoryUnite::createButton(IEventSubject* owner)
{
	return m_nativeFactory->createButton(owner);
}

ICheckBox* WidgetFactoryUnite::createCheckBox(IEventSubject* owner)
{
	return m_nativeFactory->createCheckBox(owner);
}

IComboBox* WidgetFactoryUnite::createComboBox(IEventSubject* owner)
{
	return m_nativeFactory->createComboBox(owner);
}

IContainer* WidgetFactoryUnite::createContainer(IEventSubject* owner)
{
	return m_nativeFactory->createContainer(owner);
}

IDialog* WidgetFactoryUnite::createDialog(IEventSubject* owner)
{
	return m_nativeFactory->createDialog(owner);
}

IDropDown* WidgetFactoryUnite::createDropDown(IEventSubject* owner)
{
	return m_nativeFactory->createDropDown(owner);
}

IEdit* WidgetFactoryUnite::createEdit(IEventSubject* owner)
{
	return new EditUnite(m_nativeFactory, owner);
}

IFileDialog* WidgetFactoryUnite::createFileDialog(IEventSubject* owner)
{
	return m_nativeFactory->createFileDialog(owner);
}

IForm* WidgetFactoryUnite::createForm(IEventSubject* owner)
{
	return m_nativeFactory->createForm(owner);
}

IListBox* WidgetFactoryUnite::createListBox(IEventSubject* owner)
{
	return m_nativeFactory->createListBox(owner);
}

IListView* WidgetFactoryUnite::createListView(IEventSubject* owner)
{
	return m_nativeFactory->createListView(owner);
}

IMenuBar* WidgetFactoryUnite::createMenuBar(IEventSubject* owner)
{
	return m_nativeFactory->createMenuBar(owner);
}

IMessageBox* WidgetFactoryUnite::createMessageBox(IEventSubject* owner)
{
	return m_nativeFactory->createMessageBox(owner);
}

INotificationIcon* WidgetFactoryUnite::createNotificationIcon(IEventSubject* owner)
{
	return m_nativeFactory->createNotificationIcon(owner);
}

IPanel* WidgetFactoryUnite::createPanel(IEventSubject* owner)
{
	return m_nativeFactory->createPanel(owner);
}

IPopupMenu* WidgetFactoryUnite::createPopupMenu(IEventSubject* owner)
{
	return m_nativeFactory->createPopupMenu(owner);
}

IRadioButton* WidgetFactoryUnite::createRadioButton(IEventSubject* owner)
{
	return m_nativeFactory->createRadioButton(owner);
}

IRichEdit* WidgetFactoryUnite::createRichEdit(IEventSubject* owner)
{
	return m_nativeFactory->createRichEdit(owner);
}

IScrollBar* WidgetFactoryUnite::createScrollBar(IEventSubject* owner)
{
	return m_nativeFactory->createScrollBar(owner);
}

ISlider* WidgetFactoryUnite::createSlider(IEventSubject* owner)
{
	return m_nativeFactory->createSlider(owner);
}

IStatic* WidgetFactoryUnite::createStatic(IEventSubject* owner)
{
	return m_nativeFactory->createStatic(owner);
}

IToolForm* WidgetFactoryUnite::createToolForm(IEventSubject* owner)
{
	return m_nativeFactory->createToolForm(owner);
}

ITreeView* WidgetFactoryUnite::createTreeView(IEventSubject* owner)
{
	return m_nativeFactory->createTreeView(owner);
}

IUserWidget* WidgetFactoryUnite::createUserWidget(IEventSubject* owner)
{
	return m_nativeFactory->createUserWidget(owner);
}

IWebBrowser* WidgetFactoryUnite::createWebBrowser(IEventSubject* owner)
{
	return m_nativeFactory->createWebBrowser(owner);
}

INative* WidgetFactoryUnite::createNative(IEventSubject* owner)
{
	return m_nativeFactory->createNative(owner);
}

IBitmap* WidgetFactoryUnite::createBitmap()
{
	return m_nativeFactory->createBitmap();
}

IClipboard* WidgetFactoryUnite::createClipboard()
{
	return m_nativeFactory->createClipboard();
}

 int32_t WidgetFactoryUnite::getSystemDPI() const
 {
 	return 96;
 }

bool WidgetFactoryUnite::getSystemColor(SystemColor systemColor, Color4ub& outColor)
{
	switch (systemColor)
	{
	case ScDesktopBackground:
		return m_nativeFactory->getSystemColor(systemColor, outColor);

	case ScActiveCaption:
		outColor = Color4ub(255, 220, 60);
		return true;

	case ScInactiveCaption:
		outColor = Color4ub(120, 80, 60);
		return true;

	case ScWindowText:
		outColor = Color4ub(255, 255, 255);
		return true;

	case ScMenuBackground:
		outColor = Color4ub(60, 60, 60);
		return true;

	case ScWindowBackground:
		outColor = Color4ub(40, 40, 40);
		return true;

	case ScWindowFrame:
		outColor = Color4ub(40, 40, 40);
		return true;

	case ScMenuText:
		outColor = Color4ub(255, 255, 255);
		return true;

	case ScActiveCaptionText:
		outColor = Color4ub(255, 255, 255);
		return true;

	case ScInactiveCaptionText:
		outColor = Color4ub(250, 250, 250);
		return true;

	case ScActiveBorder:
		outColor = Color4ub(120, 100, 40);
		return true;

	case ScInactiveBorder:
		outColor = Color4ub(60, 50, 40);
		return true;

	case ScButtonFaceLight:
		outColor = Color4ub(80, 80, 80);
		return true;

	case ScButtonFace:
		outColor = Color4ub(60, 60, 60);
		return true;

	case ScButtonShadow:
		outColor = Color4ub(30, 30, 30);
		return true;

	case ScDisabledText:
		outColor = Color4ub(120, 120, 120);
		return true;

	default:
		return false;
	}
}

	}
}
