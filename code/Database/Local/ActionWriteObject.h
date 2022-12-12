/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Io/Path.h"
#include "Database/Local/Action.h"

namespace traktor
{

class ChunkMemory;
class IStream;

}

namespace traktor::db
{

/*! Transaction write object action.
 * \ingroup Database
 */
class ActionWriteObject : public Action
{
	T_RTTI_CLASS;

public:
	explicit ActionWriteObject(const Path& instancePath, const std::wstring& primaryTypeName);

	virtual bool execute(Context& context) override final;

	virtual bool undo(Context& context) override final;

	virtual void clean(Context& context) override final;

	virtual bool redundant(const Action* action) const override final;

	Ref< IStream > getWriteStream() const;

	Ref< IStream > getReadStream() const;

private:
	Path m_instancePath;
	std::wstring m_primaryTypeName;
	Ref< ChunkMemory > m_objectMemory;
	bool m_editObject;
	bool m_editMeta;
};

}
