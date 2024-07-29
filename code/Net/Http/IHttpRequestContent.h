/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"

namespace traktor
{

class IStream;

}

namespace traktor::net
{

/*!
 * \ingroup Net
 */
class IHttpRequestContent : public Object
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getContentType() const = 0;

	virtual uint32_t getContentLength() const = 0;

	virtual bool encodeIntoStream(IStream* stream) const = 0;
};

}
