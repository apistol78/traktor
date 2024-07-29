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
#include "Core/Ref.h"

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
class OutputStream;

}

namespace traktor::net
{

class HttpRequest;
class HttpServerImpl;
class SocketAddressIPv4;

/*!
 * \ingroup Net
 */
class T_DLLCLASS HttpServer : public Object
{
	T_RTTI_CLASS;

public:
	class T_DLLCLASS IRequestListener : public Object
	{
		T_RTTI_CLASS;

	public:
		virtual int32_t httpClientRequest(
			HttpServer* server,
			const HttpRequest* request,
			IStream* clientStream,
			OutputStream& os,
			Ref< IStream >& outStream,
			bool& outCache,
			std::wstring& inoutSession
		) = 0;
	};

	bool create(const SocketAddressIPv4& bind);

	void destroy();

	int32_t getListenPort();

	void setRequestListener(IRequestListener* listener);

	void update(int32_t duration);

private:
	Ref< HttpServerImpl > m_impl;
};

}
