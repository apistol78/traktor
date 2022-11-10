/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#if defined(_WIN32) || defined(__LINUX__) || defined(__RPI__) || (defined(__APPLE__) && !defined(__IOS__))
#	define T_NEED_RUNTIME_SIGNATURE
#endif
