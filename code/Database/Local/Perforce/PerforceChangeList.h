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
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace db
	{

class PerforceChangeListFile;

/*! P4 change list.
 * \ingroup Database
 */
class PerforceChangeList : public ISerializable
{
	T_RTTI_CLASS;

public:
	PerforceChangeList();

	PerforceChangeList(
		uint32_t change ,
		const std::wstring& user,
		const std::wstring& client,
		const std::wstring& description
	);

	uint32_t getChange() const;

	const std::wstring& getUser() const;

	const std::wstring& getClient() const;

	const std::wstring& getDescription() const;

	void setFiles(const RefArray< PerforceChangeListFile >& files);

	const RefArray< PerforceChangeListFile >& getFiles() const;

	virtual void serialize(ISerializer& s);

private:
	uint32_t m_change;
	std::wstring m_user;
	std::wstring m_client;
	std::wstring m_description;
	RefArray< PerforceChangeListFile > m_files;
};

	}
}

