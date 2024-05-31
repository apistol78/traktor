/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Database/Remote/IMessage.h"
#include "Core/Guid.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::db
{

/*! Create instance.
 * \ingroup Database
 */
class T_DLLCLASS DbmCreateInstance : public IMessage
{
	T_RTTI_CLASS;

public:
	explicit DbmCreateInstance(uint32_t handle = 0, const std::wstring_view& name = L"", const Guid& guid = Guid());

	uint32_t getHandle() const { return m_handle; }

	const std::wstring& getName() const { return m_name; }

	const Guid& getGuid() const { return m_guid; }

	virtual void serialize(ISerializer& s) override final;

private:
	uint32_t m_handle;
	std::wstring m_name;
	Guid m_guid;
};

}
