/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Database/Local/Action.h"
#include "Core/Io/Path.h"
#include "Core/Guid.h"

namespace traktor::db
{

/*! Transaction set guid action.
 * \ingroup Database
 */
class ActionSetGuid : public Action
{
	T_RTTI_CLASS;

public:
	explicit ActionSetGuid(const Path& instancePath, const Guid& newGuid, bool create);

	virtual bool execute(Context& context) override final;

	virtual bool undo(Context& context) override final;

	virtual void clean(Context& context) override final;

	virtual bool redundant(const Action* action) const override final;

private:
	Path m_instancePath;
	Guid m_newGuid;
	bool m_create;
	bool m_editMeta;
};

}
