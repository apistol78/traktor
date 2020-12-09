#pragma once

#include <string>
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

/*! \ingroup Vulkan */
//@{

enum GlslType
{
	GtVoid = 0,
	GtBoolean = 1,
	GtInteger = 2,
	GtInteger2 = 3,
	GtInteger3 = 4,
	GtInteger4 = 5,
	GtFloat = 6,
	GtFloat2 = 7,
	GtFloat3 = 8,
	GtFloat4 = 9,
	GtFloat4x4 = 10,
	GtTexture2D = 11,
	GtTexture3D = 12,
	GtTextureCube = 13,
	GtStructBuffer = 14
};

struct GlslRequirements
{
	PrecisionHint vertexPrecisionHint;
	PrecisionHint fragmentPrecisionHint;

	GlslRequirements()
	:	vertexPrecisionHint(PhUndefined)
	,	fragmentPrecisionHint(PhUndefined)
	{
	}
};

std::wstring glsl_type_name(GlslType type);

int32_t glsl_type_width(GlslType type);

GlslType glsl_from_data_type(DataType type);

GlslType glsl_from_parameter_type(ParameterType type);

std::wstring glslStorageType(DataType type);

//@}

	}
}

