/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_GlslType_H
#define traktor_render_GlslType_H

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
	GtTextureCube
};

std::wstring glsl_vertex_attr_name(DataUsage usage, int index);

int32_t glsl_vertex_attr_location(DataUsage usage, int index);

std::wstring glsl_type_name(GlslType type);

int32_t glsl_type_width(GlslType type);

GlslType glsl_from_data_type(DataType type);

GlslType glsl_from_parameter_type(ParameterType type);

//@}

	}
}

#endif	// traktor_render_GlslType_H
