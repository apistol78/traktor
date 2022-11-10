/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
