/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/Base64.h"
#include "Core/Misc/String.h"
#include "Html/Element.h"
#include "Html/Text.h"
#include "Webber/FileUpload.h"

namespace traktor::wbr
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.FileUpload", FileUpload, Widget)

FileUpload::FileUpload(const std::wstring& text, const std::wstring& accept)
:	m_text(text)
,	m_accept(accept)
{
}

const std::wstring& FileUpload::getFileName() const
{
	return m_fileName;
}

const AlignedVector< uint8_t >& FileUpload::getData() const
{
	return m_data;
}

void FileUpload::build(html::Element* parent) const
{
	// Input has no id so its value isn't sent with other events.
	auto elabel = createElement(L"label", L"wtk_fileupload", parent);
	auto einput = new html::Element(L"input");
	einput->setAttribute(L"type", L"file");
	einput->setAttribute(L"accept", m_accept);
	einput->setAttribute(L"hidden", L"hidden");
	einput->setAttribute(L"onchange", L"post_file(this, " + toString(getId()) + L");");
	elabel->addChild(einput);
	elabel->addChild(new html::Text(m_text));
}

void FileUpload::consumeValues(const std::map< int32_t, std::wstring >& values)
{
	// Value is file name and Base64 encoded content, separated by a line feed.
	m_fileName.clear();
	m_data.clear();

	auto it = values.find(getId());
	if (it == values.end())
		return;

	const size_t p = it->second.find(L'\n');
	if (p == it->second.npos)
		return;

	m_fileName = it->second.substr(0, p);
	m_data = Base64().decode(it->second.substr(p + 1));
}

void FileUpload::consume(int32_t senderId, int32_t action)
{
	if (senderId != getId() || m_data.empty())
		return;

	m_uploaded.raise();
	m_fileName.clear();
	m_data.clear();
}

Event& FileUpload::uploaded()
{
	return m_uploaded;
}

}
