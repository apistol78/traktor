/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Misc/IHash.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/* Adler32 checksum.
 * \ingroup Core
 */
class T_DLLCLASS Adler32 : public IHash
{
	T_RTTI_CLASS;

public:
	virtual void begin() override final;

	virtual void feedBuffer(const void* buffer, uint64_t bufferSize) override final;

	virtual void end() override final;

	uint32_t get() const;

private:
	uint32_t m_A = 0;
	uint32_t m_B = 0;
	uint32_t m_feed = 0;
};

}

