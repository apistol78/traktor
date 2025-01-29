/*
 * TRAKTOR
 * Copyright (c) 2023-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"
#include "Render/Types.h"

#include <functional>
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

class T_DLLCLASS ShaderModule : public ISerializable
{
	T_RTTI_CLASS;

public:
	ShaderModule() = default;

	explicit ShaderModule(const std::wstring& text);

	void setTextDirect(const std::wstring& text);

	std::wstring escape(std::function< std::wstring(const Guid& g) > fn) const;

	const SmallMap< std::wstring, SamplerState >& getSamplers() const { return m_samplers; }

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_text;
	SmallMap< std::wstring, SamplerState > m_samplers;
};

}
