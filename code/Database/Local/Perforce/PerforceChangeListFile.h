/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/ISerializable.h"
#include "Database/Local/Perforce/PerforceTypes.h"

namespace traktor
{
	namespace db
	{

/*! P4 change list file.
 * \ingroup Database
 */
class PerforceChangeListFile : public ISerializable
{
	T_RTTI_CLASS;

public:
	PerforceChangeListFile();

	void setDepotPath(const std::wstring& depotPath);

	const std::wstring& getDepotPath() const;

	void setLocalPath(const std::wstring& localPath);

	const std::wstring& getLocalPath() const;

	void setAction(PerforceAction action);

	PerforceAction getAction() const;

	virtual void serialize(ISerializer& s);

private:
	std::wstring m_depotPath;
	std::wstring m_localPath;
	PerforceAction m_action;
};

	}
}

