#ifndef traktor_render_HlslType_H
#define traktor_render_HlslType_H

#include <string>
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

/*! \ingroup DX9 Xbox360 */
//@{

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

std::wstring hlsl_semantic(DataUsage usage, int index);

std::wstring hlsl_type_name(HlslType type);

int32_t hlsl_type_width(HlslType type);

HlslType hlsl_from_data_type(DataType type);

HlslType hlsl_from_parameter_type(ParameterType type);

//@}

	}
}

#endif	// traktor_render_HlslType_H
