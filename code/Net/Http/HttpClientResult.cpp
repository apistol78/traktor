/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/Job.h"
#include "Net/Http/HttpClientResult.h"

namespace traktor::net
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.HttpClientResult", HttpClientResult, Result)

void HttpClientResult::succeed(const HttpResponse* response, IStream* stream)
{
	m_response = response;
	m_stream = stream;
	Result::succeed();
}

const HttpResponse* HttpClientResult::getResponse()
{
	if (succeeded())
		return m_response;
	else
		return nullptr;
}

IStream* HttpClientResult::getStream()
{
	if (succeeded())
		return m_stream;
	else
		return nullptr;
}

}
