/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Reflection/RfmCompound.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Reflected array member.
 * \ingroup Core
 */
class T_DLLCLASS RfmArray : public RfmCompound
{
	T_RTTI_CLASS;

public:
	explicit RfmArray(const wchar_t* name, const Attribute* attributes);

	void insertDefault();

	uint32_t getInsertDefaultCount() const { return m_insertDefaultCount; }

private:
	uint32_t m_insertDefaultCount = 0;
};

}

