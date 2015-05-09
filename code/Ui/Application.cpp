#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/StyleSheet.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const struct { const wchar_t* name; VirtualKey vkey; } c_keyTranslateTable[] =
{
	{ L"F1", VkF1 },
	{ L"F2", VkF2 },
	{ L"F3", VkF3 },
	{ L"F4", VkF4 },
	{ L"F5", VkF5 },
	{ L"F6", VkF6 },
	{ L"F7", VkF7 },
	{ L"F8", VkF8 },
	{ L"F9", VkF9 },
	{ L"F10", VkF10 },
	{ L"F11", VkF11 },
	{ L"F12", VkF12 },
	{ L"Home", VkHome },
	{ L"End", VkEnd },
	{ L"PgUp", VkPageUp },
	{ L"PgDown", VkPageDown },
	{ L"Insert", VkInsert },
	{ L"Tab", VkTab },
	{ L"BkSpace", VkBackSpace },
	{ L"Delete", VkDelete },
	{ L"Escape", VkEscape },
	{ L"Left", VkLeft },
	{ L"Up", VkUp },
	{ L"Right", VkRight },
	{ L"Down", VkDown },
	{ L"Space", VkSpace }
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Application", Application, EventSubject)

Application* Application::getInstance()
{
	static Ref< Application > s_instance;
	if (!s_instance)
		s_instance = new Application();
	return s_instance;
}

bool Application::initialize(IEventLoop* eventLoop, IWidgetFactory* widgetFactory, StyleSheet* styleSheet)
{
	m_eventLoop = eventLoop;
	m_widgetFactory = widgetFactory;
	m_clipboard = new Clipboard(widgetFactory->createClipboard());
	
	if (styleSheet)
		m_styleSheet = styleSheet;
	else
		m_styleSheet = StyleSheet::createDefault();

	return true;
}

void Application::finalize()
{
	if (m_clipboard)
	{
		m_clipboard->destroy();
		m_clipboard = 0;
	}

	m_widgetFactory = 0;
	m_eventLoop = 0;
}

bool Application::process()
{
	T_ASSERT (m_eventLoop);
	return m_eventLoop->process(this);
}

int Application::execute()
{
	T_ASSERT (m_eventLoop);
	return m_eventLoop->execute(this);
}

void Application::exit(int exitCode)
{
	T_ASSERT (m_eventLoop);
	m_eventLoop->exit(exitCode);
}

IEventLoop* Application::getEventLoop()
{
	return m_eventLoop;
}

IWidgetFactory* Application::getWidgetFactory()
{
	return m_widgetFactory;
}

Clipboard* Application::getClipboard()
{
	return m_clipboard;
}

const StyleSheet* Application::getStyleSheet() const
{
	return m_styleSheet;
}

VirtualKey Application::translateVirtualKey(const std::wstring& keyName) const
{
	for (int i = 0; i < sizeof_array(c_keyTranslateTable); ++i)
	{
		if (compareIgnoreCase< std::wstring >(keyName, c_keyTranslateTable[i].name) == 0)
			return c_keyTranslateTable[i].vkey;
	}
	
	wchar_t ch = keyName[0];
	if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z'))
		return (VirtualKey)ch;
		
	return VkNull;
}

std::wstring Application::translateVirtualKey(VirtualKey virtualKey) const
{
	for (int i = 0; i < sizeof_array(c_keyTranslateTable); ++i)
	{
		if (virtualKey == c_keyTranslateTable[i].vkey)
			return c_keyTranslateTable[i].name;
	}
	
	if ((virtualKey >= Vk0 && virtualKey <= Vk9) || (virtualKey >= VkA && virtualKey <= VkZ))
	{
		wchar_t tmp[] = { wchar_t(virtualKey), L'\0' };
		return tmp;
	}
	
	return L"";
}

	}
}
