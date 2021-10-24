#pragma once

#include <string>
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \ingroup Render */
//@{

enum class GlslDialect
{
	OpenGL,
	Vulkan
};

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
	GtStructBuffer = 14,
	GtImage2D = 15,
	GtImage3D = 16,
	GtImageCube = 17
};

struct GlslRequirements
{
	PrecisionHint vertexPrecisionHint = PhUndefined;
	PrecisionHint fragmentPrecisionHint = PhUndefined;
};

std::wstring T_DLLCLASS glsl_type_name(GlslType type);

int32_t T_DLLCLASS glsl_type_width(GlslType type);

GlslType T_DLLCLASS glsl_promote_to_float(GlslType type);

GlslType T_DLLCLASS glsl_degrade_to_integer(GlslType type);

GlslType T_DLLCLASS glsl_precedence(GlslType typeA, GlslType typeB);

GlslType T_DLLCLASS glsl_from_data_type(DataType type);

GlslType T_DLLCLASS glsl_from_parameter_type(ParameterType type);

std::wstring T_DLLCLASS glsl_storage_type(DataType type);

std::wstring T_DLLCLASS glsl_vertex_attribute_name(DataUsage usage, int32_t index);

int32_t T_DLLCLASS glsl_vertex_attribute_location(DataUsage usage, int32_t index);

//@}

	}
}

