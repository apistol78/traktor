/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include "Core/Io/Path.h"
#include "Database/Local/Action.h"

namespace traktor::db
{

/*! Transaction set name action.
 * \ingroup Database
 */
class ActionSetName : public Action
{
	T_RTTI_CLASS;

public:
	explicit ActionSetName(const Path& instancePath, const std::wstring& newName);

	virtual bool execute(Context& context) override final;

	virtual bool undo(Context& context) override final;

	virtual void clean(Context& context) override final;

	virtual bool redundant(const Action* action) const override final;

private:
	Path m_instancePath;
	Path m_instancePathNew;
	bool m_removedMeta;
	bool m_removedObject;
	std::map< std::wstring, bool > m_removedData;
};

}
