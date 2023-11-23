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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

/*! \ingroup Render */
//@{

enum class GlslType
{
	Void = 0,
	Boolean = 1,
	Integer = 2,
	Integer2 = 3,
	Integer3 = 4,
	Integer4 = 5,
	Float = 6,
	Float2 = 7,
	Float3 = 8,
	Float4 = 9,
	Float4x4 = 10,
	Texture2D = 11,
	Texture3D = 12,
	TextureCube = 13,
	StructBuffer = 14,
	Image2D = 15,
	Image3D = 16,
	ImageCube = 17
};

struct GlslRequirements
{
	PrecisionHint vertexPrecisionHint = PrecisionHint::Undefined;
	PrecisionHint fragmentPrecisionHint = PrecisionHint::Undefined;
	int32_t localSize[3] = { 1, 1, 1 };
	bool useTargetSize = false;
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
