/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Matrix44.h"
#include "Render/SH/SHMatrix.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

/*! Generate rotation SH matrix.
 * \ingroup Render
 *
 * Generate SH rotation matrix from Cartesian rotation matrix.
 */
SHMatrix T_DLLCLASS generateRotationSHMatrix(const Matrix44& matrix, int order);

}
