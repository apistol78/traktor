/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::net
{

class HttpClientResult;
class IHttpRequestContent;
class Url;

/*!
 * \ingroup Net
 */
class T_DLLCLASS HttpClient : public Object
{
	T_RTTI_CLASS;

public:
	Ref< HttpClientResult > get(const net::Url& url, const IHttpRequestContent* content = 0);

	Ref< HttpClientResult > put(const net::Url& url, const IHttpRequestContent* content = 0);

	Ref< HttpClientResult > post(const net::Url& url, const IHttpRequestContent* content = 0);
};

}
