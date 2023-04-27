/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/Types.h"
#include "Render/Editor/Glsl/GlslResource.h"

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
	};

	explicit GlslStorageBuffer(const std::wstring& name, uint8_t stages);

	bool add(const std::wstring& elementName, DataType elementType);

	const AlignedVector< Element >& get() const { return m_elements; }

	virtual int32_t getOrdinal() const override final;

	virtual bool isBindless() const override final { return false; }

private:
	AlignedVector< Element > m_elements;
};

}
