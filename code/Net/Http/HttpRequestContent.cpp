/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Net/Url.h"
#include "Net/Http/HttpRequestContent.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.HttpRequestContent", HttpRequestContent, IHttpRequestContent)

HttpRequestContent::HttpRequestContent()
{
}

HttpRequestContent::HttpRequestContent(const std::wstring& content)
:	m_content(content)
{
}

void HttpRequestContent::set(const std::wstring& content)
{
	m_content = content;
}

std::wstring HttpRequestContent::getUrlEncodedContent() const
{
	return Url::encode(m_content);
}

	}
}
