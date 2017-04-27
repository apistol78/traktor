/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Net/File/FileConnection.h"
#include "Core/Io/FileSystem.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.FileConnection", FileConnection, UrlConnection)

UrlConnection::EstablishResult FileConnection::establish(const Url& url, Url* outRedirectionUrl)
{
	m_stream = FileSystem::getInstance().open(url.getPath(), File::FmRead);
	if (!m_stream)
		return ErNoSuchResource;

	m_url = url;
	return ErSucceeded;
}

Url FileConnection::getUrl() const
{
	return m_url;
}

Ref< IStream > FileConnection::getStream()
{
	return m_stream;
}

	}
}
