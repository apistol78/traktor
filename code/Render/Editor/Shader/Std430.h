/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Object.h"
#include "Core/Ref.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class StructDeclaration;

class T_DLLCLASS Std430 : public Object
{
	T_RTTI_CLASS;

public:
	static Ref< Std430 > create(const StructDeclaration* decl);

	uint32_t getSize() const { return m_size; }

	int32_t getElementIndex(const std::wstring& element) const;

	int32_t getElementOffset(const std::wstring& element) const;

	std::wstring format(const std::wstring& element, const void* data) const;

private:
	Ref< const StructDeclaration > m_decl;
	AlignedVector< int32_t > m_offsets;
	uint32_t m_size = 0;
};

}
