/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/Editor/Glsl/GlslResource.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class T_DLLCLASS GlslStorageBuffer : public GlslResource
{
	T_RTTI_CLASS;

public:
	struct Element
	{
		std::wstring name;
		DataType type;
		int32_t length;
	};

	explicit GlslStorageBuffer(const std::wstring& name, const std::wstring& structTypeNam, Set set, uint8_t stages, bool indexed);

	const std::wstring& getStructTypeName() const { return m_structTypeName; }

	bool isIndexed() const { return m_indexed; }

	virtual int32_t getClassOrdinal() const override final { return 3; }

	virtual int32_t getOrdinal() const override final;

private:
	// AlignedVector< Element > m_elements;
	std::wstring m_structTypeName;
	bool m_indexed;
};

}
