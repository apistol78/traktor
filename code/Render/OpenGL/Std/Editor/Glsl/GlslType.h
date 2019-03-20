#pragma once

#include <string>
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

/*! \ingroup OGL */
//@{

enum GlslType
{
	GtVoid,
	GtFloat,
	GtFloat2,
	GtFloat3,
	GtFloat4,
	GtFloat4x4,
	GtTexture2D,
	GtTexture3D,
	GtTextureCube,
	GtStructBuffer
};

struct GlslRequirements
{
	bool derivatives;
	bool transpose;
	bool texture3D;
	bool shadowSamplers;
	bool vertexBilinearSampler;
	PrecisionHint precisionHint;

	GlslRequirements()
	:	derivatives(false)
	,	transpose(false)
	,	texture3D(false)
	,	shadowSamplers(false)
	,	vertexBilinearSampler(false)
	,	precisionHint(PhUndefined)
	{
	}
};

std::wstring glsl_type_name(GlslType type);

int32_t glsl_type_width(GlslType type);

GlslType glsl_from_data_type(DataType type);

GlslType glsl_from_parameter_type(ParameterType type);

//@}

	}
}

