#include "Core/Io/StringOutputStream.h"
#include "Webber/Utilities.h"

namespace traktor
{
	namespace wbr
	{
	
std::wstring escapeJson(const std::wstring& text)
{
	StringOutputStream ss;
	for (auto c : text)
	{
		switch (c)
		{
		case L'\t':
			ss << L"\\t";
			break;
		case L'\n':
			ss << L"\\n";
			break;
		case L'\r':
			ss << L"\\r";
			break;
		case L'\"':
			ss << L"\\\"";
			break;
		case L'\'':
			ss << L"\\\'";
			break;
		case L'\\':
			ss << L"\\\\";
			break;
		default:
			if (c >= 0x20)
				ss << c;
			else
				ss << L' ';
			break;
		}
	}
	return ss.str();
}

std::wstring escapeHtml(const std::wstring& text)
{
	StringOutputStream ss;
	for (wchar_t c : text)
	{
		switch (c)
		{
		case L'&':
			ss << L"&amp;";
			break;

		case L'<':
			ss << L"&lt;";
			break;

		case L'>':
			ss << L"&gt;";
			break;

		case L'\"':
			ss << L"&quot;";
			break;

		case L'\'':
			ss << L"&#039;";
			break;

		case L'\n':
			ss << L"<br/>";
			break;

		case L'\r':
			break;

		case L'\t':
			ss << L"&nbsp;&nbsp;&nbsp;&nbsp;";
			break;

		default:
			ss << c;
			break;
		}
	}
	return ss.str();
 }

	}
}
