#ifndef traktor_render_HlslType_H
#define traktor_render_HlslType_H

#include <string>
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup DX10
 */
enum HlslType
{
	HtVoid,
	HtBoolean,
	HtFloat,
	HtFloat2,
	HtFloat3,
	HtFloat4,
	HtFloat4x4
};

/*!
 * \ingroup DX10
 */
std::wstring hlsl_semantic(DataUsage usage, int index);

/*!
 * \ingroup DX10
 */
std::wstring hlsl_type_name(HlslType type);

/*!
 * \ingroup DX10
 */
HlslType hlsl_from_data_type(DataType type);

	}
}

#endif	// traktor_render_HlslType_H
