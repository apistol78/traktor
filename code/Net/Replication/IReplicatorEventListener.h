/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class Replicator;
class ReplicatorProxy;

class T_DLLCLASS IReplicatorEventListener : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool notify(
		Replicator* replicator,
		float eventTime,
		ReplicatorProxy* fromProxy,
		const Object* eventObject
	) = 0;
};

	}
}

