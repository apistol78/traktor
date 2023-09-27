/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Net/UrlConnection.h"
#include "Net/Url.h"
#include "Net/Http/HttpConnection.h"
#include "Net/Ftp/FtpConnection.h"
#include "Net/File/FileConnection.h"

namespace traktor::net
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.UrlConnection", UrlConnection, Object)

Ref< UrlConnection > UrlConnection::open(const Url& url)
{
	if (!url.valid())
		return nullptr;

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
			connection = nullptr;
	}

	return connection;
}

}
