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
	GtFloat = 3,
	GtFloat2 = 4,
	GtFloat3 = 5,
	GtFloat4 = 6,
	GtFloat4x4 = 7,
	GtTexture2D = 8,
	GtTexture3D = 9,
	GtTextureCube = 10,
	GtStructBuffer = 11
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

