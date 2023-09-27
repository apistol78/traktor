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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::net
{

/*! Network manager.
 * \ingroup Net
 */
class T_DLLCLASS Network
{
public:
	/*! Initialize network.
	 * \return True if network is initialized and ready to be used.
	 */
	static bool initialize();

	/*! Finalize network. */
	static void finalize();

private:
	static int32_t ms_initialized;
};

}
