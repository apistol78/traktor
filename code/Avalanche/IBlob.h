/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Date/DateTime.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AVALANCHE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

}

namespace traktor::avalanche
{

class T_DLLCLASS IBlob : public Object
{
	T_RTTI_CLASS;

public:
	virtual int64_t size() const = 0;

	virtual Ref< IStream > append() = 0;

	virtual Ref< IStream > read() const = 0;

	virtual bool remove() = 0;

	virtual bool touch() = 0;

	virtual DateTime lastAccessed() const = 0;
};

}
