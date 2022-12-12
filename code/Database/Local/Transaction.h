/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"

namespace traktor
{

class Mutex;

}

namespace traktor::db
{

class Action;
class Context;

/*! Transaction
 * \ingroup Database
 */
class Transaction : public Object
{
	T_RTTI_CLASS;

public:
	Transaction();

	virtual ~Transaction();

	bool create(const Guid& transactionGuid);

	void destroy();

	void add(Action* action);

	bool commit(Context& context);

	uint32_t get(const TypeInfo& actionType, RefArray< Action >& outActions) const;

	template < typename ActionType >
	uint32_t get(RefArray< ActionType >& outActions) const
	{
		return get(type_of< ActionType >(), (RefArray< Action >&)outActions);
	}

private:
	Ref< Mutex > m_lock;
	RefArray< Action > m_actions;
	Guid m_transactionGuid;
#if defined(_DEBUG)
	bool m_locked;
#endif
};

}
