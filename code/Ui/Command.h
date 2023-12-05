/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

/*! Command
 * \ingroup UI
 */
class T_DLLCLASS Command : public Object
{
	T_RTTI_CLASS;

public:
	enum CommandFlags
	{
		CfNone = 0,
		CfId = 1,
		CfName = 2,
		CfData = 4
	};

	Command();

	explicit Command(uint32_t id);

	explicit Command(const std::wstring_view& name);

	explicit Command(uint32_t id, const std::wstring_view& name);

	explicit Command(const std::wstring_view& name, Object* data);

	explicit Command(uint32_t id, const std::wstring_view& name, Object* data);

	uint32_t getFlags() const;

	uint32_t getId() const;

	const std::wstring& getName() const;

	Ref< Object > getData() const;

	bool operator == (const Command& command) const;

	bool operator == (uint32_t id) const;

	bool operator == (const std::wstring_view& name) const;

private:
	uint32_t m_flags;
	uint32_t m_id;
	std::wstring m_name;
	Ref< Object > m_data;
};

}
