/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

/*! Persistent script.
 * \ingroup Script
 */
class T_DLLCLASS Script : public ISerializable
{
	T_RTTI_CLASS;

public:
	Script() = default;

	explicit Script(const std::wstring& text);

	void setTextDirect(const std::wstring& text);

	std::wstring escape(std::function< std::wstring (const Guid& g) > fn) const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_text;
};

	}
}

