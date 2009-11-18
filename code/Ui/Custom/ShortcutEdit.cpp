#include "Ui/Custom/ShortcutEdit.h"
#include "Ui/Application.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/KeyEvent.h"
#include "Ui/Events/PaintEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int32_t c_preferedWidth = 100;
const int32_t c_preferedHeight = 18;

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ShortcutEdit", ShortcutEdit, Widget)

ShortcutEdit::ShortcutEdit()
:	m_keyState(0)
,	m_virtualKey(VkNull)
{
}

bool ShortcutEdit::create(Widget* parent, int32_t keyState, VirtualKey virtualKey, int style)
{
	if (!Widget::create(parent, style))
		return false;

	m_keyState = keyState;
	m_virtualKey = virtualKey;

	addKeyDownEventHandler(createMethodHandler(this, &ShortcutEdit::eventKeyDown));
	addPaintEventHandler(createMethodHandler(this, &ShortcutEdit::eventPaint));
	addFocusEventHandler(createMethodHandler(this, &ShortcutEdit::eventFocus));

	return true;
}

std::wstring ShortcutEdit::getText() const
{
	if (m_virtualKey == VkNull)
		return L"";

	std::wstring keyDesc = L"";

	if (m_keyState & ui::KsControl)
		keyDesc = L"Ctrl";
	if (m_keyState & ui::KsMenu)
		keyDesc = keyDesc.empty() ? L"Alt" : keyDesc + L"+Alt";
	if (m_keyState & ui::KsShift)
		keyDesc = keyDesc.empty() ? L"Shift" : keyDesc + L"+Shift";

	std::wstring keyName = ui::Application::getInstance()->translateVirtualKey(m_virtualKey);
	return keyDesc.empty() ? keyName : keyDesc + L", " + keyName;
}

Size ShortcutEdit::getPreferedSize() const
{
	return Size(c_preferedWidth, c_preferedHeight);
}

void ShortcutEdit::set(int32_t keyState, VirtualKey virtualKey)
{
	m_keyState = keyState;
	m_virtualKey = virtualKey;
}

int32_t ShortcutEdit::getKeyState() const
{
	return m_keyState;
}

VirtualKey ShortcutEdit::getVirtualKey() const
{
	return m_virtualKey;
}

void ShortcutEdit::addChangeEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiContentChange, eventHandler);
}

void ShortcutEdit::eventKeyDown(Event* event)
{
	KeyEvent* keyEvent = checked_type_cast< KeyEvent* >(event);

	VirtualKey virtualKey = keyEvent->getVirtualKey();
	
	/*
	if (virtualKey == 16 || virtualKey == 17)
	{
		keyEvent->consume();
		return;
	}
	*/

	m_keyState = keyEvent->getKeyState();
	m_virtualKey = virtualKey;

	update();

	Event changeEvent(this, 0);
	raiseEvent(EiContentChange, &changeEvent);

	keyEvent->consume();
}

void ShortcutEdit::eventPaint(Event* event)
{
	PaintEvent* paintEvent = checked_type_cast< PaintEvent* >(event);
	Canvas& canvas = paintEvent->getCanvas();

	Rect rc = getInnerRect();

	if (!hasFocus())
		canvas.setBackground(getSystemColor(ScWindowBackground));
	else
		canvas.setBackground(Color(220, 255, 220));

	canvas.fillRect(rc);

	std::wstring text = getText();
	canvas.drawText(rc.inflate(-8, 0), text, AnLeft, AnCenter);
	
	event->consume();
}

void ShortcutEdit::eventFocus(Event* event)
{
	update();
}

		}
	}
}
