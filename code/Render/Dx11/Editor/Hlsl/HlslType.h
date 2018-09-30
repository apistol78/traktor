/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_HlslType_H
#define traktor_render_HlslType_H

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
	HtVoid,
	HtBoolean,
	HtFloat,
	HtFloat2,
	HtFloat3,
	HtFloat4,
	HtFloat4x4,
	HtTexture2D,
	HtTexture3D,
	HtTextureCube
};

/*!
 * \ingroup DX11
 */
std::wstring hlsl_semantic(DataUsage usage, int index);

/*!
 * \ingroup DX11
 */
std::wstring hlsl_type_name(HlslType type, bool lowPrecision);

/*!
 * \ingroup DX11
 */
int32_t hlsl_type_width(HlslType type);

/*!
 * \ingroup DX11
 */
HlslType hlsl_from_data_type(DataType type);

/*!
 * \ingroup DX11
 */
HlslType hlsl_from_parameter_type(ParameterType type);

	}
}

#endif	// traktor_render_HlslType_H
