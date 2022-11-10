/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

}

namespace traktor::db
{

class Group;
class Instance;

/*! Instance isolation.
 * \ingroup Database
 */
struct T_DLLCLASS Isolate
{
	static bool createIsolatedInstance(Instance* instance, IStream* stream);

	static Ref< Instance > createInstanceFromIsolation(Group* group, IStream* stream);
};

}
