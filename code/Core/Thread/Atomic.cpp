/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#if defined(_DEBUG)
#	if defined(_WIN32)
#		include "Core/Thread/Win32/Atomic.inl"
#	elif defined(__APPLE__)
#		include "Core/Thread/OsX/Atomic.inl"
#	else
#		include "Core/Thread/Linux/Atomic.inl"
#	endif
#endif
