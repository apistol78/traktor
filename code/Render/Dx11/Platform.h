/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#define _WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#if defined(T_USE_D3DPERF)
#	include <d3d9.h>
#endif
#include <d3d11.h>
#include <d3dcompiler.h>
#include <tchar.h>

