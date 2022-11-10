/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include "Database/Remote/IMessage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

/*! Handle array result.
 * \ingroup Database
 */
class T_DLLCLASS MsgGetChildrenResult : public IMessage
{
	T_RTTI_CLASS;

public:
	void addGroup(uint32_t handle);

	void addInstance(uint32_t handle);

	const std::vector< uint32_t >& getGroups() const;

	const std::vector< uint32_t >& getInstances() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::vector< uint32_t > m_groupHandles;
	std::vector< uint32_t > m_instanceHandles;
};

	}
}

