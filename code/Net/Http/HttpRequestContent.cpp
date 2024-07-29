/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Net/Url.h"
#include "Net/Http/HttpRequestContent.h"

namespace traktor::net
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.HttpRequestContent", HttpRequestContent, IHttpRequestContent)

HttpRequestContent::HttpRequestContent(const std::wstring& content)
{
	set(content);
}

HttpRequestContent::HttpRequestContent(IStream* stream)
{
	set(stream);
}

void HttpRequestContent::set(const std::wstring& content)
{
	const std::string cu8 = wstombs(
		Utf8Encoding(),
		content
	);

	if (false /* url encoded */)
	{
		const std::string ue = wstombs(
			Utf8Encoding(),
			Url::encode((const uint8_t*)cu8.c_str(), cu8.size())
		);

		m_contentType = L"application/x-www-form-urlencoded";
		m_content.insert(
			m_content.end(),
			(const uint8_t*)ue.c_str(),
			(const uint8_t*)ue.c_str() + ue.size()
		);
	}
	else
	{
		m_contentType = L"application/octet-stream";
		m_content.insert(
			m_content.end(),
			(const uint8_t*)cu8.c_str(),
			(const uint8_t*)cu8.c_str() + cu8.size()
		);
	}
}

void HttpRequestContent::set(IStream* stream)
{
	DynamicMemoryStream dms(m_content, false, true);
	if (StreamCopy(&dms, stream).execute())
		m_contentType = L"application/octet-stream";
	else
	{
		m_content.clear();
		m_contentType = L"";
	}
	
}

std::wstring HttpRequestContent::getContentType() const
{
	return m_contentType;
}

uint32_t HttpRequestContent::getContentLength() const
{
	return (uint32_t)m_content.size();
}

bool HttpRequestContent::encodeIntoStream(IStream* stream) const
{
	return stream->write(m_content.c_ptr(), m_content.size()) == m_content.size();
}

}
