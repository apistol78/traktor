/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Net/Ftp/FtpConnection.h"
#include "Net/Ftp/FtpClient.h"
#include "Net/Url.h"
#include "Net/SocketAddressIPv6.h"
#include "Core/Io/IStream.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.FtpConnection", FtpConnection, UrlConnection)

FtpConnection::~FtpConnection()
{
	if (m_stream)
		m_stream->close();

	if (m_client)
		m_client->disconnect();
}

UrlConnection::EstablishResult FtpConnection::establish(const Url& url, Url* outRedirectionUrl)
{
	if (url.getProtocol() != L"ftp")
		return ErInvalidUrl;
	
	SocketAddressIPv6 address(url.getHost(), url.getPort());

	// Connect to server.
	m_client = new FtpClient();
	if (!m_client->connect(address))
		return ErConnectFailed;

	// Get user and password from URL user info.
	std::wstring userInfo = url.getUserInfo();
	if (!userInfo.empty())
	{
		size_t i = userInfo.find(':');
		if (i != userInfo.npos)
		{
			std::wstring user = userInfo.substr(0, i);
			std::wstring password = userInfo.substr(i + 1);
			if (!m_client->login(user, password))
				return ErLoginFailed;
		}
		else
		{
			if (!m_client->login(userInfo, L""))
				return ErLoginFailed;
		}
	}
	else
	{
		if (!m_client->login(L"anonymous", L""))
			return ErLoginFailed;
	}

	// Open stream to resource.
	m_stream = m_client->open(url.getFile());
	if (!m_stream)
		return ErNoSuchResource;

	m_url = url;
	return ErSucceeded;
}

Url FtpConnection::getUrl() const
{
	return m_url;
}

Ref< IStream > FtpConnection::getStream()
{
	return m_stream;
}

	}
}
