/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Object.h"
#include "Net/Url.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

	namespace net
	{

/*! URL connection base.
 * \ingroup Net
 */
class T_DLLCLASS UrlConnection : public Object
{
	T_RTTI_CLASS;

public:
	enum EstablishResult
	{
		ErSucceeded = 0,
		ErRedirect = 1,
		ErInvalidUrl = -1,
		ErConnectFailed = -2,
		ErLoginFailed = -3,
		ErNoSuchResource = -4,
		ErFailed = -100
	};

	/*! Open connection from URL.
	 *
	 * \param url URL to open.
	 * \return Established connection; null if unable to connect.
	 */
	static Ref< UrlConnection > open(const Url& url);

	/*! Establish connection from URL.
	 *
	 * \note
	 * You should use open method rather than calling this
	 * method directly as this might return a redirection URL
	 * which need to be followed in order to establish a
	 * connection.
	 *
	 * \param url URL to open.
	 * \param outRedirectionUrl Redirection URL.
	 * \return Establish result.
	 */
	virtual EstablishResult establish(const Url& url, Url* outRedirectionUrl) = 0;

	/*! Get URL of connection.
	 *
	 * \return URL of connection.
	 */
	virtual Url getUrl() const = 0;

	/*! Get stream to resource.
	 *
	 * \return Stream to resource.
	 */
	virtual Ref< IStream > getStream() = 0;
};

	}
}

