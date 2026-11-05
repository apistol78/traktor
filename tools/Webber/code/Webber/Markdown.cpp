/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cwchar>
#include <cwctype>
#include "Core/Containers/AlignedVector.h"
#include "Core/Misc/String.h"
#include "Html/Element.h"
#include "Html/Text.h"
#include "Webber/Markdown.h"

namespace traktor::wbr
{
	namespace
	{

typedef AlignedVector< std::wstring > lines_t;

void parseBlocks(const lines_t& lines, html::Element* parent, bool tight);

html::Element* addElement(html::Element* parent, const std::wstring& name)
{
	auto element = new html::Element(name);
	parent->addChild(element);
	return element;
}

void flushText(std::wstring& text, html::Element* parent)
{
	if (!text.empty())
	{
		parent->addChild(new html::Text(text));
		text.clear();
	}
}

bool isEscapable(wchar_t ch)
{
	return ch != 0 && std::wcschr(L"\\`*_{}[]()#+-.!>", ch) != nullptr;
}

bool isSafeUrl(const std::wstring& url)
{
	// Only known schemes, so script urls cannot be injected; relative urls have no scheme.
	const size_t colon = url.find(L':');
	if (colon == url.npos || url.find_first_of(L"/?#") < colon)
		return true;
	const std::wstring scheme = toLower(url.substr(0, colon));
	return scheme == L"http" || scheme == L"https" || scheme == L"mailto";
}

size_t runLength(const std::wstring& text, size_t from)
{
	size_t end = from;
	while (end < text.size() && text[end] == text[from])
		++end;
	return end - from;
}

size_t findEmphasisClose(const std::wstring& text, size_t from, wchar_t ch, size_t count)
{
	for (size_t i = from; i < text.size(); )
	{
		if (text[i] == L'\\')
		{
			i += 2;
			continue;
		}
		if (text[i] != ch)
		{
			++i;
			continue;
		}

		// Closing delimiter is the end of a run which follows text.
		const size_t end = i + runLength(text, i);
		if (end - i >= count && i > from && !std::iswspace(text[i - 1]) && (ch != L'_' || end >= text.size() || !std::iswalnum(text[end])))
			return end - count;
		i = end;
	}
	return text.npos;
}

size_t findBracketClose(const std::wstring& text, size_t from, wchar_t open, wchar_t close)
{
	int32_t depth = 0;
	for (size_t i = from; i < text.size(); ++i)
	{
		if (text[i] == L'\\')
			++i;
		else if (text[i] == open)
			++depth;
		else if (text[i] == close && --depth == 0)
			return i;
	}
	return text.npos;
}

void parseInline(const std::wstring& text, html::Element* parent)
{
	std::wstring buffer;
	size_t i = 0;

	while (i < text.size())
	{
		const wchar_t ch = text[i];

		if (ch == L'\\' && i + 1 < text.size() && isEscapable(text[i + 1]))
		{
			buffer += text[i + 1];
			i += 2;
		}
		else if (ch == L'`')
		{
			const size_t n = runLength(text, i);
			const size_t close = text.find(std::wstring(n, L'`'), i + n);
			if (close != text.npos)
			{
				flushText(buffer, parent);
				addElement(parent, L"code")->addChild(new html::Text(trim(text.substr(i + n, close - i - n))));
				i = close + n;
			}
			else
			{
				buffer.append(n, L'`');
				i += n;
			}
		}
		else if (ch == L'*' || ch == L'_')
		{
			const size_t n = runLength(text, i);
			const size_t count = std::min< size_t >(n, 2);
			const bool canOpen = i + count < text.size() && !std::iswspace(text[i + count]) && (ch != L'_' || i == 0 || !std::iswalnum(text[i - 1]));
			const size_t close = canOpen ? findEmphasisClose(text, i + count, ch, count) : text.npos;
			if (close != text.npos)
			{
				flushText(buffer, parent);
				parseInline(text.substr(i + count, close - i - count), addElement(parent, count == 2 ? L"strong" : L"em"));
				i = close + count;
			}
			else
			{
				buffer.append(n, ch);
				i += n;
			}
		}
		else if (ch == L'[' || (ch == L'!' && i + 1 < text.size() && text[i + 1] == L'['))
		{
			const bool image = (ch == L'!');
			const size_t labelOpen = image ? i + 1 : i;
			const size_t labelClose = findBracketClose(text, labelOpen, L'[', L']');
			const size_t urlClose = (labelClose != text.npos && labelClose + 1 < text.size() && text[labelClose + 1] == L'(') ? findBracketClose(text, labelClose + 1, L'(', L')') : text.npos;
			if (urlClose != text.npos)
			{
				const std::wstring label = text.substr(labelOpen + 1, labelClose - labelOpen - 1);
				std::wstring url = trim(text.substr(labelClose + 2, urlClose - labelClose - 2));
				url = url.substr(0, url.find(L' '));

				flushText(buffer, parent);
				if (isSafeUrl(url) && image)
				{
					auto img = addElement(parent, L"img");
					img->setAttribute(L"src", url);
					img->setAttribute(L"alt", label);
					img->setAttribute(L"loading", L"lazy");
				}
				else if (isSafeUrl(url))
				{
					auto a = addElement(parent, L"a");
					a->setAttribute(L"href", url);
					parseInline(label, a);
				}
				else
					parseInline(label, parent);
				i = urlClose + 1;
			}
			else
			{
				buffer += ch;
				++i;
			}
		}
		else if (ch == L'\n')
		{
			// Two trailing spaces or a backslash breaks the line.
			if (endsWith(buffer, L"  ") || endsWith(buffer, L"\\"))
			{
				buffer = rtrim(buffer, L" \\");
				flushText(buffer, parent);
				addElement(parent, L"br");
			}
			else
				buffer += ch;
			++i;
		}
		else
		{
			buffer += ch;
			++i;
		}
	}

	flushText(buffer, parent);
}

size_t indentOf(const std::wstring& line)
{
	return std::min(line.find_first_not_of(L' '), line.size());
}

bool isBlank(const std::wstring& line)
{
	return indentOf(line) == line.size();
}

bool isFence(const std::wstring& line)
{
	return startsWith(ltrim(line), L"```");
}

int32_t headingLevel(const std::wstring& line)
{
	const size_t p = indentOf(line);
	const size_t n = (p < line.size() && line[p] == L'#') ? runLength(line, p) : 0;
	return (n >= 1 && n <= 6 && (p + n == line.size() || line[p + n] == L' ')) ? (int32_t)n : 0;
}

bool isRule(const std::wstring& line)
{
	const std::wstring t = trim(line);
	if (t.empty() || std::wcschr(L"-*_", t[0]) == nullptr)
		return false;
	int32_t count = 0;
	for (auto c : t)
	{
		if (c == t[0])
			++count;
		else if (c != L' ')
			return false;
	}
	return count >= 3;
}

bool isQuote(const std::wstring& line)
{
	const size_t p = indentOf(line);
	return p < line.size() && line[p] == L'>';
}

/*! Offset of item content after list marker, 0 if line isn't a list item. */
size_t listMarker(const std::wstring& line, bool& outOrdered)
{
	const size_t p = indentOf(line);
	if (p >= line.size())
		return 0;

	size_t q = p;
	if (line[p] == L'-' || line[p] == L'*' || line[p] == L'+')
	{
		outOrdered = false;
		++q;
	}
	else
	{
		while (q < line.size() && std::iswdigit(line[q]) && q - p < 9)
			++q;
		if (q == p || q >= line.size() || (line[q] != L'.' && line[q] != L')'))
			return 0;
		outOrdered = true;
		++q;
	}

	if (q == line.size())
		return q;
	return line[q] == L' ' ? q + 1 : 0;
}

bool startsBlock(const std::wstring& line)
{
	bool ordered;
	return isFence(line) || headingLevel(line) > 0 || isRule(line) || isQuote(line) || listMarker(line, ordered) > 0;
}

size_t nextNonBlank(const lines_t& lines, size_t from)
{
	while (from < lines.size() && isBlank(lines[from]))
		++from;
	return from;
}

size_t parseList(const lines_t& lines, size_t start, html::Element* parent)
{
	bool ordered = false;
	listMarker(lines[start], ordered);
	const size_t indent = indentOf(lines[start]);

	AlignedVector< lines_t > items;
	bool tight = true;
	size_t i = start;

	while (i < lines.size() && !isRule(lines[i]))
	{
		bool itemOrdered = false;
		const size_t content = listMarker(lines[i], itemOrdered);
		if (content == 0 || itemOrdered != ordered || indentOf(lines[i]) > indent + 1)
			break;

		lines_t& item = items.push_back();
		item.push_back(lines[i].substr(content));

		for (++i; i < lines.size(); ++i)
		{
			const std::wstring& line = lines[i];
			if (isBlank(line))
			{
				// Blank line continues the item only when indented content follows.
				const size_t j = nextNonBlank(lines, i);
				if (j >= lines.size() || indentOf(lines[j]) <= indent)
					break;
				tight = false;
				item.push_back(L"");
			}
			else if (indentOf(line) > indent)
				item.push_back(line.substr(std::min(indentOf(line), content)));
			else if (!startsBlock(line))
				item.push_back(line);
			else
				break;
		}

		// Blank lines between items make the list loose.
		const size_t j = nextNonBlank(lines, i);
		if (j > i && j < lines.size())
		{
			bool nextOrdered = false;
			if (listMarker(lines[j], nextOrdered) > 0 && nextOrdered == ordered && indentOf(lines[j]) <= indent + 1 && !isRule(lines[j]))
			{
				tight = false;
				i = j;
			}
		}
	}

	auto list = addElement(parent, ordered ? L"ol" : L"ul");
	for (const auto& item : items)
		parseBlocks(item, addElement(list, L"li"), tight);

	return i;
}

void parseBlocks(const lines_t& lines, html::Element* parent, bool tight)
{
	size_t i = 0;
	while (i < lines.size())
	{
		const std::wstring& line = lines[i];
		bool ordered;

		if (isBlank(line))
			++i;
		else if (isFence(line))
		{
			std::wstring code;
			for (++i; i < lines.size() && !isFence(lines[i]); ++i)
				code += (code.empty() ? L"" : L"\n") + lines[i];
			++i;
			addElement(parent, L"pre")->addChild(new html::Text(code));
		}
		else if (const int32_t level = headingLevel(line))
		{
			parseInline(trim(line.substr(indentOf(line) + level)), addElement(parent, L"h" + toString(level)));
			++i;
		}
		else if (isRule(line))
		{
			addElement(parent, L"hr");
			++i;
		}
		else if (isQuote(line))
		{
			lines_t quoted;
			for (; i < lines.size() && isQuote(lines[i]); ++i)
			{
				const std::wstring q = lines[i].substr(indentOf(lines[i]) + 1);
				quoted.push_back(startsWith(q, L" ") ? q.substr(1) : q);
			}
			parseBlocks(quoted, addElement(parent, L"blockquote"), false);
		}
		else if (listMarker(line, ordered) > 0)
			i = parseList(lines, i, parent);
		else
		{
			std::wstring text;
			for (; i < lines.size() && !isBlank(lines[i]) && (text.empty() || !startsBlock(lines[i])); ++i)
				text += (text.empty() ? L"" : L"\n") + ltrim(lines[i], L" ");
			parseInline(rtrim(text, L" "), tight ? parent : addElement(parent, L"p"));
		}
	}
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.Markdown", Markdown, Widget)

Markdown::Markdown(const std::wstring& text)
:	m_text(text)
{
}

void Markdown::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& Markdown::getText() const
{
	return m_text;
}

void Markdown::build(html::Element* parent) const
{
	// Leading tabs are expanded so indentation can be measured in spaces.
	lines_t lines;
	for (size_t p = 0; p <= m_text.size(); )
	{
		size_t e = m_text.find(L'\n', p);
		if (e == m_text.npos)
			e = m_text.size();

		std::wstring line;
		size_t c = p;
		for (; c < e && (m_text[c] == L' ' || m_text[c] == L'\t'); ++c)
			line.append(m_text[c] == L'\t' ? 4 : 1, L' ');
		line += m_text.substr(c, e - c);
		lines.push_back(rtrim(line, L"\r"));

		p = e + 1;
	}

	auto ediv = createElement(L"div", L"wtk_markdown", parent);
	parseBlocks(lines, ediv, false);
}

}
