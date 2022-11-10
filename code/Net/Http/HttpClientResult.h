/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Thread/Result.h"

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
class Job;

	namespace net
	{

class HttpResponse;

/*! \brief
 * \ingroup Net
 */
class T_DLLCLASS HttpClientResult : public Result
{
	T_RTTI_CLASS;

public:
	void succeed(const HttpResponse* response, IStream* stream);

	const HttpResponse* getResponse();

	IStream* getStream();

private:
	friend class HttpClient;

	Ref< Job > m_job;
	Ref< const HttpResponse > m_response;
	Ref< IStream > m_stream;
};

	}
}
