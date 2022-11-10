/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Database/Events/EvtInstance.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

/*! Instance committed event.
 * \ingroup Database
 */
class T_DLLCLASS EvtInstanceCommitted : public EvtInstance
{
	T_RTTI_CLASS;

public:
	EvtInstanceCommitted() = default;

	explicit EvtInstanceCommitted(const Guid& instanceGuid);
};

	}
}

