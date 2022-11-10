/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup DX11
 */
enum HlslType
{
	HtVoid = 0,
	HtBoolean = 1,
	HtInteger = 2,
	HtInteger2 = 3,
	HtInteger3 = 4,
	HtInteger4 = 5,
	HtFloat = 6,
	HtFloat2 = 7,
	HtFloat3 = 8,
	HtFloat4 = 9,
	HtFloat4x4 = 10,
	HtTexture2D = 11,
	HtTexture3D = 12,
	HtTextureCube = 13,
	HtStructBuffer = 14
};

/*!
 * \ingroup DX11
 */
std::wstring hlsl_semantic(DataUsage usage, int index);

/*!
 * \ingroup DX11
 */
std::wstring hlsl_type_name(HlslType type);

/*!
 * \ingroup DX11
 */
int32_t hlsl_type_width(HlslType type);


/*!
 * \ingroup DX11
 */
HlslType hlsl_promote_to_float(HlslType type);

/*!
 * \ingroup DX11
 */
HlslType hlsl_degrade_to_integer(HlslType type);

/*!
 * \ingroup DX11
 */
HlslType hlsl_precedence(HlslType typeA, HlslType typeB);

/*!
 * \ingroup DX11
 */
HlslType hlsl_from_data_type(DataType type);

/*!
 * \ingroup DX11
 */
HlslType hlsl_from_parameter_type(ParameterType type);

/*!
 * \ingroup DX11
 */
std::wstring hlsl_storage_type(DataType type);

	}
}

