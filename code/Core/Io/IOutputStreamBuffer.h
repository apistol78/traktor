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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Buffer used by output streams.
 * \ingroup Core
 */
class T_DLLCLASS IOutputStreamBuffer : public Object
{
	T_RTTI_CLASS;

public:
	virtual int32_t getIndent() const = 0;

	virtual void setIndent(int32_t indent) = 0;

	virtual int32_t getDecimals() const = 0;

	virtual void setDecimals(int32_t decimals) = 0;

	virtual bool getPushIndent() const = 0;

	virtual void setPushIndent(bool pushIndent) = 0;

	virtual int32_t overflow(const wchar_t* buffer, int32_t count) = 0;
};

}

