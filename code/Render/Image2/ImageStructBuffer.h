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
#include "Core/Object.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImageStructBuffer : public Object
{
	T_RTTI_CLASS;

public:
	explicit ImageStructBuffer(const std::wstring& name, handle_t persistentHandle, uint32_t elementCount, uint32_t elementSize);

	const std::wstring& getName() const;

	handle_t getPersistentHandle() const;

	uint32_t getElementCount() const { return m_elementCount; }

	uint32_t getElementSize() const { return m_elementSize; }

private:
	std::wstring m_name;
	handle_t m_persistentHandle;
	uint32_t m_elementCount;
	uint32_t m_elementSize;
};

}
