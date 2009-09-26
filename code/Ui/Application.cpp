#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Core/Heap/GcNew.h"
#include "Core/Misc/StringUtilities.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const struct { const wchar_t* name; int code; } c_keyTranslateTable[] =
{
	{ L"F1", 112 },
	{ L"F2", 113 },
	{ L"F3", 114 },
	{ L"F4", 115 },
	{ L"F5", 116 },
	{ L"F6", 117 },
	{ L"F7", 118 },
	{ L"F8", 119 },
	{ L"F9", 120 },
	{ L"Home", 36 },
	{ L"End", 35 },
	{ L"PgUp", 33 },
	{ L"PgDown", 34 },
	{ L"Insert", 45 },
	{ L"Tab", 9 },
	{ L"BkSpace", 8 },
	{ L"Delete", 46 },
	{ L"Escape", 27 },
	{ L"Left", 37 },
	{ L"Up", 38 },
	{ L"Right", 39 },
	{ L"Down", 40 },
	{ L"Space", 32 }
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Application", Application, EventSubject)

Application& Application::getInstance()
{
	static Application instance;
	return instance;
}

bool Application::initialize(IEventLoop* eventLoop, IWidgetFactory* widgetFactory)
{
	m_eventLoop = eventLoop;
	m_widgetFactory = widgetFactory;
	m_clipboard = gc_new< Clipboard >(widgetFactory->createClipboard());
	return true;
}

void Application::finalize()
{
	if (m_clipboard)
	{
		m_clipboard->destroy();

		Heap::getInstance().invalidateRefs(m_clipboard);
		T_ASSERT (m_clipboard == 0);
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

int Application::translateVirtualKey(const std::wstring& keyName) const
{
	for (int i = 0; i < sizeof_array(c_keyTranslateTable); ++i)
	{
		if (compareIgnoreCase(keyName, c_keyTranslateTable[i].name) == 0)
			return c_keyTranslateTable[i].code;
	}
	return int(keyName[0]);
}

std::wstring Application::translateVirtualKey(int keyCode) const
{
	for (int i = 0; i < sizeof_array(c_keyTranslateTable); ++i)
	{
		if (keyCode == c_keyTranslateTable[i].code)
			return c_keyTranslateTable[i].name;
	}
	wchar_t tmp[] = { wchar_t(keyCode), L'\0' };
	return tmp;
}

	}
}
