/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Net/UrlConnection.h"
#include "Net/Url.h"
#include "Net/Http/HttpConnection.h"
#include "Net/Ftp/FtpConnection.h"
#include "Net/File/FileConnection.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.UrlConnection", UrlConnection, Object)

Ref< UrlConnection > UrlConnection::open(const Url& url)
{
	if (!url.valid())
		return 0;

	Ref< UrlConnection > connection;

	if (url.getProtocol() == L"http")
		connection = new HttpConnection();
	else if (url.getProtocol() == L"ftp")
		connection = new FtpConnection();
	else if (url.getProtocol() == L"file")
		connection = new FileConnection();

	if (connection)
	{
		Url redirectionUrl;
		EstablishResult result = connection->establish(url, &redirectionUrl);
		if (result == ErRedirect)
			connection = UrlConnection::open(redirectionUrl);
		else if (result != ErSucceeded)
			connection = 0;
	}

	return connection;
}

	}
}
